#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>

#define TAPE_SIZE 131072

enum Instruction {
    INSTR_LEFT,
    INSTR_RIGHT,
    INSTR_INC,
    INSTR_DEC,
    INSTR_IN,
    INSTR_OUT,
    INSTR_LOOP,
    INSTR_LOOPEND,
};

void panic(const char *str)
{
    assert(str != nullptr);
    printf("Error: %s\n", str);
    exit(-1);
}

void runtime_error(const char *str, int pc)
{
    assert(str != nullptr);
    printf("Runtime error at (%i): %s\n", pc, str);
    exit(-1);
}

void execute(uint8_t *instr, int instr_len)
{
    uint8_t memory[TAPE_SIZE];
    int pc = 0;
    int head = 0;

    while (pc < instr_len)
    {
        switch (instr[pc])
        {
        case INSTR_RIGHT:
            head += 1;
            if (head >= TAPE_SIZE) head = 0;
            ++pc;
            break;

        case INSTR_LEFT:
            head -= 1;
            if (head < 0) head = TAPE_SIZE - 1;
            ++pc;
            break;

        case INSTR_INC:
            memory[head] += 1;
            ++pc;
            break;

        case INSTR_DEC:
            memory[head] -= 1;
            ++pc;
            break;

        case INSTR_OUT:
            putchar((int)(memory[head]));
            ++pc;
            break;

        case INSTR_IN:
            memory[head] = (uint8_t)(getchar());
            ++pc;
            break;

        case INSTR_LOOP:
            if (memory[head] == 0)
            {
                // Look forwards to find matching ']'
                int bal = 1;
                while (bal != 0)
                {
                    ++pc;
                    if (pc >= instr_len)
                    {
                        runtime_error("EOF reached while trying to find matching []s", pc);
                    }

                    if (instr[pc] == INSTR_LOOPEND) bal -= 1;
                    if (instr[pc] == INSTR_LOOP) bal += 1;
                }
            }
            else
            {
                ++pc;
            }
            break;

        case INSTR_LOOPEND:
            if (memory[head] != 0)
            {
                // Look backwards to find matching '['
                int bal = -1;
                while (bal != 0)
                {
                    --pc;
                    if (pc < 0) runtime_error("EOF reached while trying to find matching []s", pc);

                    if (instr[pc] == INSTR_LOOPEND) bal -= 1;
                    if (instr[pc] == INSTR_LOOP) bal += 1;
                }
            }
            else
            {
                ++pc;
            }
            break;
        }
    }
}

int main(int argc, const char *argv[])
{
    const char *err_file = "File manipulation error";
    const char *err_mem = "Memory allocation failed";

    if (argc != 2)
    {
        printf("Usage: bf.exe <filename>\n");
        exit(-1);
    }

    // Read the contents of the file and convert it into a list of instructions
    FILE *f = fopen(argv[1], "rb");
    if (f == nullptr) panic("Could not open file");

    if (fseek(f, 0, SEEK_END) != 0) panic(err_file);
    long file_size = ftell(f);
    if (file_size == -1) panic(err_file);

    long buf_size = file_size + 1;

    char *buf = (char *)malloc(sizeof(char) * buf_size);
    if (buf == nullptr) panic(err_mem);

    if (fseek(f, 0, SEEK_SET) != 0) panic(err_file);
    if (fread(buf, sizeof(char), buf_size - 1, f) !=  buf_size - 1) panic(err_file);

    buf[buf_size - 1] = '\0';

    fclose(f);

    uint8_t *instr = (uint8_t *)malloc(sizeof(uint8_t) * buf_size);
    if (instr == nullptr) panic(err_mem);
    int instr_len = 0;

    for (int i = 0; i < buf_size; ++i)
    {
        switch (buf[i])
        {
        case '>':
            instr[instr_len] = INSTR_RIGHT;
            ++instr_len;
            break;

        case '<':
            instr[instr_len] = INSTR_LEFT;
            ++instr_len;
            break;

        case '+':
            instr[instr_len] = INSTR_INC;
            ++instr_len;
            break;

        case '-':
            instr[instr_len] = INSTR_DEC;
            ++instr_len;
            break;

        case '.':
            instr[instr_len] = INSTR_OUT;
            ++instr_len;
            break;

        case ',':
            instr[instr_len] = INSTR_IN;
            ++instr_len;
            break;

        case '[':
            instr[instr_len] = INSTR_LOOP;
            ++instr_len;
            break;

        case ']':
            instr[instr_len] = INSTR_LOOPEND;
            ++instr_len;
            break;
        }
    }

    free(buf); // The file buffer is no longer needed

    execute(instr, instr_len);

    free(instr);

    return 0;
}
