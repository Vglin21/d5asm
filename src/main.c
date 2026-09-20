#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dot-5/cpu.h>

#ifdef _WIN32
  #include <locale.h>
#endif

#ifdef _WIN32
  #define ASM_NAME "d5asm.exe"
#else
  #define ASM_NAME "d5asm"
#endif
#define MAX_LABEL_LEN 64
#define MAX_LINE_LEN 512
#define MAX_SYMBOLS 248
#define MAX_LABELS 128

bool error = false;
bool asm_error = false;
bool line_read = false;

char line[MAX_LINE_LEN] = {0};
word line_count = 0;
word lpos = 0;

#ifdef _WIN32
wchar_t *src_file = NULL;
#else
char *src_file = NULL;
#endif

char serr[MAX_LINE_LEN];

void logerr(const char *msg) {
#ifdef _WIN32
    wprintf(L"%ls:%d:%d: error: %s\n", src_file, line_count, lpos, msg);
#else
    printf("%s:%d:%d: error: %s\n", src_file, line_count, lpos, msg);
#endif

    if (line[strlen(line)-1] == '\n') printf("| %s", line);
    else printf("| %s\n", line);

    memset(serr, 0, MAX_LINE_LEN);
    memset(serr, ' ', lpos);
    printf("| %s^\n", serr);

    error = true;
}

typedef struct {
    char name[4];
    byte no_arg;
    byte imm;
    byte zp;
    byte type;
} Opcode;

typedef enum {
    OT_BRANCH = 1
} OpcodeType;

typedef struct {
    char name[9];
    qword arg;
    char label[MAX_LABEL_LEN+1];
    byte flags;
} Symbol;

typedef enum {
    SF_HAS_ARG = 1,
    SF_IMMEDIATE = 2
} SymbolFlag;

typedef struct {
    char str[MAX_LABEL_LEN+1];
    byte address;
} Label;

const Opcode opcodes[] = {
    {"INC", INC, 0,     0,     0},
    {"DEC", DEC, 0,     0,     0},
    {"LDA", 0,   LDA_I, LDA_Z, 0},
    {"STA", 0,   0,     STA,   0},
    {"JMP", 0,   JMP,   JMP,   0},
    {"BEQ", 0,   BEQ,   BEQ,   1},
    {"BNE", 0,   BNE,   BNE,   1},
    {"ADD", 0,   ADD_I, ADD_Z, 0},
    {"SUB", 0,   SUB_I, SUB_Z, 0},
    {"ORA", 0,   ORA_I, ORA_Z, 0},
    {"AND", 0,   AND_I, AND_Z, 0}
};

const char *keywords[] = {
    ".ORG",
    ".BYTE",
    ".WORD",
    ".DWORD",
    ".QWORD"
};

Symbol symbols[MAX_SYMBOLS];
word symbol_count = 0;

Label labels[MAX_LABELS];
word label_count = 0;

byte bin[ROM_SIZE] = {0};
byte bin_addr = 0;
byte bin_count = 0;

void to_big_letters(char *str) {
    for (dword c = 0; c < strlen(str); ++c)
        if ('a' <= str[c] && str[c] <= 'z') str[c] -= 0x20;
}

bool is_digit(char num) {
    return '0' <= num && num <= '9';
}

bool is_letter(char ch) {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}

bool is_hex(char hex) {
    return is_digit(hex) || ('a' <= hex && hex <= 'f') || ('A' <= hex && hex <= 'F');
}

bool is_binary(char b) {
    return b == '0' || b == '1';
}

bool is_eol() {
    if (lpos >= MAX_LINE_LEN) return true;

    char ch = line[lpos];
    return ch == '\n' || ch == '\r' || ch == '\0';
}

bool is_empty() {
    if (is_eol()) return true;
    else return line[lpos] == ' ';
}

void skip_line() { while (!is_eol()) ++lpos; }

void skip_space() { while (!is_eol() && line[lpos] == ' ') ++lpos; }

qword read_hex() {
    char hex[17] = {0};
    byte hsize = 0;
    byte shift = 0;
    qword ret = 0;
    
    skip_space();
    
    while (!is_eol() && hsize < 16 && is_hex(line[lpos]))
        hex[hsize++] = line[lpos++];
    
    if (!is_eol() && is_hex(line[lpos])) {
        logerr("hex value is too long");
        while (!is_eol() && is_hex(line[lpos])) ++lpos;
    }
    
    if (!is_empty() && line[lpos] != ',') {
        logerr("unexpected symbol");
        while (!is_empty()) ++lpos;
    }

    if (error) return 0;

    for (byte c = hsize - 1; c < hsize;) {
        char h = hex[c--];

        if ('0' <= h && h <= '9') ret += (h - '0') << shift;
        else if ('a' <= h && h <= 'f') ret += (h - 'a' + 0xa) << shift;
        else ret += (h - 'A' + 0xa) << shift;

        shift += 4;
    }
    return (qword)ret;
}

qword read_binary() {
    char binary[65] = {0};
    byte bsize = 0;
    byte shift = 0;
    qword ret = 0;

    skip_space();

    while (!is_eol() && bsize < 64 && is_binary(line[lpos]))
        binary[bsize++] = line[lpos++];
    
    if (!is_eol() && is_binary(line[lpos])) {
        logerr("binary value is too long");
        while (!is_eol() && is_binary(line[lpos])) ++lpos;
    }
    
    if (!is_empty() && line[lpos] != ',') {
        logerr("unexpected symbol");
        while (!is_empty()) ++lpos;
    }

    if (error) return 0;

    for (byte c = bsize - 1; c < bsize;) 
        ret += (binary[c--] - '0') << (shift++);
    
    return (qword)ret;
}

qword read_decimal() {
    char num[33] = {0};
    
    byte c = 0;
    while (c < 32 && is_digit(line[lpos]) && lpos < MAX_LINE_LEN)
        num[c++] = line[lpos++];
    
    if (lpos < MAX_LINE_LEN && is_digit(line[lpos])) {
        logerr("decimal value is too long");
        while (!is_eol() && is_digit(line[lpos])) ++lpos;
    }
    
    if (!is_empty() && line[lpos] != ',') {
        logerr("unexpected symbol");
        while (!is_empty()) ++lpos;
    }

    if (error) return 0;
    
    return (qword)atoi(num);
}

void read_symbol_name(Symbol *symb) {
    memset(symb->name, 0, 9);

    byte c = 0;
    if (line[lpos] == '.')
        symb->name[c++] = line[lpos++];

    while (!is_eol() && c < 8 && is_letter(line[lpos]))
        symb->name[c++] = line[lpos++];
    
    to_big_letters(symb->name);
    
    if (!is_empty()) {
        if (is_letter(line[lpos])) {
            if (symb->name[0] == '.') logerr("keyword is too long");
            else logerr("operation name is too long");
        } else logerr("unexpected symbol");
        
        while (!is_empty()) ++lpos;
        return;
    }
    
    if (symb->name[0] == '.') {
        for (c = 0; c < (sizeof(keywords) / sizeof(char*));)
            if (!strcmp(keywords[c++], symb->name)) return;
        logerr("unrecognized keyword");
    } else {
        for (c = 0; c < (sizeof(opcodes) / sizeof(Opcode));) {
            Opcode opcode = opcodes[c++];
            if (!strcmp(opcode.name, symb->name)) return;
        }
        logerr("unrecognized operation");
    }
}

void read_symbol_value(Symbol *symb) {
    symb->flags |= SF_HAS_ARG;
    if (line[lpos] == '$') {
        ++lpos;

        if (is_empty()) {
            logerr("expected a value");
            return;
        }

        symb->arg = read_hex();
    } else if (line[lpos] == '%') {
        ++lpos;

        if (is_empty()) {
            logerr("expected a value");
            return;
        }

        symb->arg = read_binary();
    } else symb->arg = read_decimal();
}

void read_symbol() {
    Symbol *symb = &symbols[symbol_count];
    memset(symb, 0, sizeof(Symbol));

    read_symbol_name(symb);
    
    skip_space();

    if (symb->name[0] == '.') {
        if (is_eol()) logerr("no value given");
        else {
            while (1) {
                char ch = line[lpos];
                if (ch == '$' || ch == '%' || is_digit(ch)) read_symbol_value(symb);
                else {
                    logerr("unexpected symbol");
                    while (!is_empty()) ++lpos;
                    break;
                }
                
                if (!strcmp(symb->name, ".ORG")) bin_addr = symb->arg;
                else if (!strcmp(symb->name, ".BYTE")) ++bin_addr;
                else if (!strcmp(symb->name, ".WORD")) bin_addr += 2;
                else if (!strcmp(symb->name, ".DWORD")) bin_addr += 4;
                else if (!strcmp(symb->name, ".QWORD")) bin_addr += 8;
                
                ++symbol_count;
                
                skip_space();
                
                if (!is_eol() && line[lpos] == ',') {
                    ++lpos;

                    if (!strcmp(symb->name, ".ORG")) {
                        logerr("can't take more than one value");
                        break;
                    }

                    skip_space();

                    symb = &symbols[symbol_count];
                    strcpy(symb->name, symbols[symbol_count-1].name);
                } else break;
            }
        }
    } else {
        if (is_eol()) {
            ++bin_addr;
            ++symbol_count;
        } else {
            if (line[lpos] == '#') {
                symb->flags |= SF_IMMEDIATE;
                ++lpos;
            }
    
            if (is_empty()) logerr("expected a value");
            else if (line[lpos] == '$' || line[lpos] == '%' || is_digit(line[lpos])) read_symbol_value(symb);
            else if (is_letter(line[lpos])) {
                byte c = 0;
                while (!is_eol() && (is_letter(line[lpos]) || is_digit(line[lpos])) && c < MAX_LABEL_LEN)
                    symb->label[c++] = line[lpos++];
                
                symb->label[c] = '\0';
                symb->flags |= SF_HAS_ARG;
    
                if (!is_empty()) {
                    logerr("unexpected symbol");
                    while (!is_empty()) ++lpos;
                }
            } else {
                logerr("unexpected symbol");
                while (!is_empty()) ++lpos;
            }
        
            if (!error) {
                bin_addr += 2;
                ++symbol_count;
            }
        }
    }

    line_read = true;
}

void read_label() {
    Label *label = &labels[label_count];
    memset(label, 0, sizeof(Label));

    byte c = 0;
    while (!is_eol() && (is_letter(line[lpos]) || is_digit(line[lpos])) && c < MAX_LABEL_LEN)
        label->str[c++] = line[lpos++];
    label->str[c] = '\0';

    skip_space();

    if (!is_eol()) {
        if (line[lpos] != '=' && line[lpos] != ':') {
            logerr("unexpected symbol");
            while (!is_empty()) ++lpos;
        } else {
            if (line[lpos] == '=') {
                ++lpos;
                skip_space();
    
                if (line[lpos] == '$') {
                    ++lpos;

                    if (is_empty()) {
                        logerr("expected a value");
                        line_read = true;
                        return;
                    }

                    label->address = read_hex();
                } else if (line[lpos] == '%') {
                    ++lpos;

                    if (is_empty()) {
                        logerr("expected a value");
                        line_read = true;
                        return;
                    }

                    label->address = read_binary();
                } else if (is_digit(line[lpos])) label->address = read_decimal();
                else {
                    logerr("unexpected symbol");
                    while (!is_empty()) ++lpos;
                }

                line_read = true;
            } else {
                label->address = bin_addr;
                ++lpos;
            }
        }
    } else label->address = bin_addr;

    if (!error) ++label_count;
}

void to_bin(Symbol symb) {
    if (symb.name[0] == '.') {
        // for fuck's sake it's 2 AM and i'm SLEEPY give me some slack
        if (!strcmp(symb.name, ".ORG")) {
            if (bin_count != 0) bin_count = (symb.arg - bin_addr);
            bin_addr = (symb.arg - bin_addr);
        } else if (!strcmp(symb.name, ".BYTE")) {
            bin_addr++; bin[bin_count++] = symb.arg;
        } else if (!strcmp(symb.name, ".WORD")) {
            for (byte shift = 0; shift < 16; shift += 8) {
                bin_addr++; bin[bin_count++] = (byte)(symb.arg >> shift);
            }
        } else if (!strcmp(symb.name, ".DWORD")) {
            for (byte shift = 0; shift < 32; shift += 8) {
                bin_addr++; bin[bin_count++] = (byte)(symb.arg >> shift);
            }
        } else if (!strcmp(symb.name, ".QWORD")) {
            for (byte shift = 0; shift < 64; shift += 8) {
                bin_addr++; bin[bin_count++] = (byte)(symb.arg >> shift);
            }
        }
    } else {
        for (byte c = 0; c < (sizeof(opcodes) / sizeof(Opcode)) && bin_count < ROM_SIZE; ++c) {
            if (!strcmp(symb.name, opcodes[c].name)) {
                Opcode opcode = opcodes[c];
                if (!(symb.flags & SF_HAS_ARG)) {
                    bin_addr++; bin[bin_count++] = opcode.no_arg;
                    return;
                }

                if (symb.label[0] != '\0') {
                    bool found = false;

                    for (byte i = 0; i < label_count; ++i) {
                        if (!strcmp(symb.label, labels[i].str)) {
                            if (opcode.type & OT_BRANCH)
                                symb.arg = labels[i].address - bin_addr - 2;
                            else symb.arg = labels[i].address;

                            found = true;
                            
                            break;
                        }
                    }

                    if (!found) {
                        printf(ASM_NAME": error: label \"%s\" is undefined\n", symb.label);
                        asm_error = true;
                    }
                }

                if (symb.flags & SF_IMMEDIATE) { bin_addr++; bin[bin_count++] = opcode.imm; }
                else  { bin_addr++; bin[bin_count++] = opcode.zp; }

                if (bin_count < ROM_SIZE)  { bin_addr++; bin[bin_count++] = symb.arg; }
            }
        }
    }
}

#ifdef _WIN32

#define char wchar_t
#define s(a) L##a
#define sfmt "%ls"

#define main wmain
#define strcmp(a, b) wcscmp(a, b)
#define fopen(a, b) _wfopen(a, b)

#else
  #define s(a) a
  #define sfmt "%s"
#endif

int main(int argc, char *argv[]) {
#ifdef _WIN32
    setlocale(LC_ALL, "");
#endif

    char *out_file = s("output.d5");

    for (byte c = 1; c < argc; ++c) {
        if (!strcmp(argv[c], s("-h")) || !strcmp(argv[c], s("--help"))) {
            printf(
                "Usage: "ASM_NAME" [flags] file\n"
                "Options:\n"
                "  -h --help - Display this message.\n"
                "  -o <file> - Place the output into <file>.\n"
            );
            return 0;
        } else if (!strcmp(argv[c], s("-o"))) {
            if (++c >= argc) {
                printf(ASM_NAME": error: missing filename after \"-o\"\n");
                error = true;
            } else out_file = argv[c];
        } else if (src_file != NULL)
            printf(ASM_NAME": warning: more than one file are given, any file after \""sfmt"\" will be skipped\n", src_file);
        else src_file = argv[c];
    }

    FILE *file;
    if (!src_file) {
        printf(ASM_NAME": error: no input file\n");
        error = true;
    } else {
        if (!(file = fopen(src_file, s("r")))) {
            printf(ASM_NAME": error: couldn't open \""sfmt"\"\n", src_file);
            error = true;
        }
    }

    if (error) {
        printf(ASM_NAME": assembly terminated\n");
        if (file) fclose(file);
        return 1;
    }
    
    while (fgets(line, 256, file)) {
        bool nl = true;
        lpos = 0;
        ++line_count;
        error = false;
        line_read = false;
        
        while (!is_eol()) {
            char ch = line[lpos];

            if (is_empty()) ++lpos;
            else if (is_letter(ch)) {
                if (nl)
                    read_label();
                else if (line_read) {
                    logerr("can't take more than one operand");
                    while (!is_empty()) ++lpos;
                } else
                    read_symbol();
            } else if (ch == ';') skip_line();
            else if (ch == '.' && !nl && !line_read) read_symbol();
            else {
                logerr("unexpected symbol");
                while (!is_empty()) ++lpos;
            }

            nl = false;
            if (error) asm_error = true;
        }
    }

    fclose(file);

    bin_addr = 0;
    for (byte c = 0; c < symbol_count; ++c) to_bin(symbols[c]);

    if (error || asm_error) {
        printf(ASM_NAME": assembly terminated\n");
        return 1;
    } else {
        if (!(file = fopen(out_file, s("wb")))) return 1;
        
        fwrite(bin, 1, ROM_SIZE, file);
        printf(ASM_NAME": %d bytes written\n", bin_count);
    
        fclose(file);
    }

    return 0;
}