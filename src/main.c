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

#define SYMBOL_INIT_ALLOC 256
#define LABEL_INIT_ALLOC 128
#define BIN_INIT_ALLOC 256

bool error = false;
bool asm_error = false;
bool line_read = false;
bool nl = false;

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
    fwprintf(stderr, L"%ls:%d:%d: error: %s\n", src_file, line_count, lpos, msg);
#else
    fprintf(stderr, "%s:%d:%d: error: %s\n", src_file, line_count, lpos, msg);
#endif

    if (line[strlen(line)-1] == '\n') fprintf(stderr, "| %s", line);
    else fprintf(stderr, "| %s\n", line);

    memset(serr, 0, MAX_LINE_LEN);
    memset(serr, ' ', lpos);
    fprintf(stderr, "| %s^\n", serr);

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
    OT_HAS_ARG = 1,
    OT_BRANCH = 2
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
    char name[MAX_LABEL_LEN+1];
    qword value;
    char label[MAX_LABEL_LEN+1];
} Label;

const Opcode opcodes[] = {
    {"INC", INC, 0,     0,     0b00000000},
    {"DEC", DEC, 0,     0,     0b00000000},
    {"LDA", 0,   LDA_I, LDA_Z, 0b00000001},
    {"STA", 0,   0,     STA,   0b00000001},
    {"JMP", 0,   JMP,   JMP,   0b00000001},
    {"BEQ", 0,   BEQ,   BEQ,   0b00000011},
    {"BNE", 0,   BNE,   BNE,   0b00000011},
    {"ADD", 0,   ADD_I, ADD_Z, 0b00000001},
    {"SUB", 0,   SUB_I, SUB_Z, 0b00000001},
    {"ORA", 0,   ORA_I, ORA_Z, 0b00000001},
    {"AND", 0,   AND_I, AND_Z, 0b00000001}
};

const char *keywords[] = {
    ".ORG",
    ".BYTE",
    ".WORD",
    ".DWORD",
    ".QWORD"
};

dword symbol_alloc = SYMBOL_INIT_ALLOC;
Symbol *symbols;
dword symbol_count = 0;

dword label_alloc = LABEL_INIT_ALLOC;
Label *labels;
dword label_count = 0;

dword bin_alloc = BIN_INIT_ALLOC;
byte *bin;
dword bin_addr = 0;
dword bin_count = 0;

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

    byte shift = 0;
    qword ret = 0;

    for (byte c = hsize - 1; c < hsize;) {
        char h = hex[c--];

        if ('0' <= h && h <= '9') ret += (h - '0') << shift;
        else if ('a' <= h && h <= 'f') ret += (h - 'a' + 0xa) << shift;
        else ret += (h - 'A' + 0xa) << shift;

        shift += 4;
    }
    return ret;
}

qword read_binary() {
    char binary[65] = {0};
    byte bsize = 0;

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

    byte shift = 0;
    qword ret = 0;

    for (byte c = bsize - 1; c < bsize;) 
        ret += (binary[c--] - '0') << (shift++);
    return ret;
}

qword read_decimal() {
    char num[65] = {0};
    
    byte c = 0;
    while (!is_eol() && c < 64 && is_digit(line[lpos]))
        num[c++] = line[lpos++];
    
    if (!is_eol() && is_digit(line[lpos])) {
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

void read_name(char *dest, dword count) {
    byte c = 0;
    while (!is_eol() && (is_letter(line[lpos]) || is_digit(line[lpos])) && c < count)
        dest[c++] = line[lpos++];
    dest[c] = '\0';

    if (!is_eol() && (is_letter(line[lpos]) || is_digit(line[lpos]))) {
        logerr("label is too long");
        while (!is_eol() && (is_letter(line[lpos]) || is_digit(line[lpos]))) ++lpos;
    }
    
    if (!is_empty()) {
        if ((line[lpos] == ':' || line[lpos] == '=') && !nl) {
            logerr("unexpected symbol");
            while (!is_empty()) ++lpos;
        }
    }
}

qword read_value() {
    if (line[lpos] == '$') {
        ++lpos;

        if (is_empty()) {
            logerr("expected a hex value");
            return 0;
        }

        if (!is_hex(line[lpos])) {
            logerr("expected a hex value");
            return 0;
        } else return read_hex();
    } else if (line[lpos] == '%') {
        ++lpos;

        if (is_empty()) {
            logerr("expected a binary value");
            return 0;
        }

        if (!is_binary(line[lpos])) {
            logerr("expected a binary value");
            return 0;
        } else return read_binary();
    } else return read_decimal();
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
            if (symb->name[0] == '.')
                logerr("keyword is too long");
            else
                logerr("operation name is too long");
        } else logerr("unexpected symbol");
        
        while (!is_empty()) ++lpos;
        return;
    }
    
    if (symb->name[0] == '.') {
        for (c = 0; c < (sizeof(keywords) / sizeof(char*));)
            if (!strcmp(keywords[c++], symb->name)) return;
            
        logerr("unrecognized keyword");
    } else {
        for (c = 0; c < (sizeof(opcodes) / sizeof(Opcode));)
            if (!strcmp(opcodes[c++].name, symb->name)) return;
        
        logerr("unrecognized operation");
    }
}

void read_symbol() {
    if (!symbols) return;

    Symbol *symb = &symbols[symbol_count];
    memset(symb, 0, sizeof(Symbol));

    read_symbol_name(symb); 
    skip_space();

    if (symb->name[0] == '.') {
        if (is_eol()) logerr("no value given");
        else {
            while (true) {
                char ch = line[lpos];
                if (ch == '$' || ch == '%' || is_digit(ch)) {
                    symb->arg = read_value();
                    symb->flags |= SF_HAS_ARG;
                } else if (is_letter(ch)) {
                    read_name(symb->label, MAX_LABEL_LEN);
                    symb->flags |= SF_HAS_ARG;
                } else {
                    logerr("unexpected symbol");
                    while (!is_empty()) ++lpos;
                    break;
                }
                
                if (!strcmp(symb->name, ".ORG")) {
                    if (symb->label[0] != '\0') logerr("can't use labels on .org");
                    else bin_addr = symb->arg;
                } else if (!strcmp(symb->name, ".BYTE")) ++bin_addr;
                else if (!strcmp(symb->name, ".WORD")) bin_addr += 2;
                else if (!strcmp(symb->name, ".DWORD")) bin_addr += 4;
                else if (!strcmp(symb->name, ".QWORD")) bin_addr += 8;
                
                ++symbol_count;
                skip_space();
                
                if (!is_eol() && line[lpos] == ',') {
                    if (!strcmp(symb->name, ".ORG")) {
                        logerr("can't take more than one value");
                        ++lpos;
                        break;
                    }
                    
                    ++lpos;
                    skip_space();

                    symb = &symbols[symbol_count];
                    strcpy(symb->name, symbols[symbol_count-1].name);
                } else break;
            }
        }
    } else {
        if (is_eol()) {
            bool has_arg = false;
            for (word c = 0; c < (sizeof(opcodes) / sizeof(Opcode)); ++c) {
                if (!strcmp(opcodes[c].name, symb->name)) {
                    has_arg = opcodes[c].type & OT_HAS_ARG;
                    break;
                }
            }

            if (has_arg) logerr("expected an operand");
            else {
                ++bin_addr;
                ++symbol_count;
    
                if (symbol_count >= symbol_alloc) {
                    symbol_alloc <<= 1;
                    symbols = (Symbol*)realloc(symbols, symbol_alloc * sizeof(Symbol));
                }
            }
        } else {
            if (line[lpos] == '#') {
                symb->flags |= SF_IMMEDIATE;
                ++lpos;
            }
    
            char ch = line[lpos];
            if (is_empty()) logerr("expected a value");
            else if (ch == '$' || ch == '%' || is_digit(ch)) {
                symb->arg = read_value();
                symb->flags |= SF_HAS_ARG;
            } else if (is_letter(ch)) {
                read_name(symb->label, MAX_LABEL_LEN);
                symb->flags |= SF_HAS_ARG;
            } else {
                logerr("unexpected symbol");
                while (!is_empty()) ++lpos;
            }
        
            if (!error) {
                bin_addr += 2;
                ++symbol_count;

                if (symbol_count >= symbol_alloc) {
                    symbol_alloc <<= 1;
                    symbols = (Symbol*)realloc(symbols, symbol_alloc * sizeof(Symbol));
                }
            }
        }
    }

    line_read = true;
}

void read_label() {
    if (!labels) return;

    Label *label = &labels[label_count];
    memset(label, 0, sizeof(Label));

    read_name(label->name, MAX_LABEL_LEN);
    if (error) return;

    skip_space();

    if (!is_eol()) {
        if (line[lpos] != '=' && line[lpos] != ':') {
            logerr("unexpected symbol");
            while (!is_empty()) ++lpos;
        } else {
            if (line[lpos] == '=') {
                ++lpos;
                skip_space();
    
                char ch = line[lpos];
                if (ch == '$' || ch == '%' || is_digit(ch)) label->value = read_value();
                else if (is_letter(ch)) read_name(label->label, MAX_LABEL_LEN);
                else {
                    logerr("unexpected symbol");
                    while (!is_empty()) ++lpos;
                }

                line_read = true;
            } else {
                label->value = bin_addr;
                ++lpos;
            }
        }
    } else label->value = bin_addr;

    if (!error) {
        ++label_count;
    
        if (label_count >= label_alloc) {
            label_alloc <<= 1;
            labels = (Label*)realloc(labels, label_alloc * sizeof(Label));
        }
    }
}

qword find_label(const char *label, bool is_branch) {
    bool found = false;
    qword ret = 0;

    for (byte i = 0; i < label_count; ++i) {
        if (!strcmp(label, labels[i].name)) {
            if (is_branch)
                ret = labels[i].value - bin_addr - 2;
            else ret = labels[i].value;

            found = true;
            break;
        }
    }

    if (!found) {
        fprintf(stderr, ASM_NAME": error: label \"%s\" was referenced but never defined\n", label);
        asm_error = true;
    }

    return ret;
}

void to_bin(Symbol symb) {
    if (symb.name[0] == '.') {
        if (!strcmp(symb.name, ".ORG")) {
            if (symb.label[0] != '\0')
                symb.arg = find_label(symb.label, false);

            if (bin_count != 0) {
                word bc = bin_count + (int64_t)(symb.arg - bin_addr);

                if (bc >= bin_alloc) {
                    while (bc >= bin_alloc) bin_alloc <<= 1;

                    bin = (byte*)realloc(bin, bin_alloc);
                    if (!bin) return;
                }
                
                memset(&bin[bin_count], 0, bin_alloc - bin_count);
                bin_count = bc;
            }
            bin_addr = symb.arg;
        } else {
            byte bit_size = 0;

            if (!strcmp(symb.name, ".BYTE")) bit_size = 8;
            else if (!strcmp(symb.name, ".WORD")) bit_size = 16;
            else if (!strcmp(symb.name, ".DWORD")) bit_size = 32;
            else if (!strcmp(symb.name, ".QWORD")) bit_size = 64;

            if (bit_size) {
                if (symb.label[0] != '\0')
                    symb.arg = find_label(symb.label, false);

                for (byte shift = 0; shift < bit_size; shift += 8) {
                    bin_addr++; bin[bin_count++] = (byte)(symb.arg >> shift);
                }

                if (bin_count >= bin_alloc) {
                    bin_alloc <<= 1;
                    bin = (byte*)realloc(bin, bin_alloc);
                }
                if (!bin) return;
            }
        }
    } else {
        for (byte c = 0; c < (sizeof(opcodes) / sizeof(Opcode)); ++c) {
            if (!strcmp(symb.name, opcodes[c].name)) {
                Opcode opcode = opcodes[c];
                if (!(symb.flags & SF_HAS_ARG)) {
                    bin_addr++; bin[bin_count++] = opcode.no_arg;
                    return;
                }

                if (symb.label[0] != '\0')
                    symb.arg = find_label(symb.label, opcode.type & OT_BRANCH);

                if (symb.flags & SF_IMMEDIATE) { bin_addr++; bin[bin_count++] = opcode.imm; }
                else  { bin_addr++; bin[bin_count++] = opcode.zp; }

                if (bin_count >= bin_alloc) {
                    bin_alloc <<= 1;
                    bin = (byte*)realloc(bin, bin_alloc);
                }
                if (!bin) return;

                bin_addr++; bin[bin_count++] = symb.arg; 
            
                if (bin_count >= bin_alloc) {
                    bin_alloc <<= 1;
                    bin = (byte*)realloc(bin, bin_alloc);
                }
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
                fprintf(stderr, ASM_NAME": error: missing filename after \"-o\"\n");
                error = true;
            } else out_file = argv[c];
        } else if (src_file != NULL)
            fprintf(stderr, ASM_NAME": warning: more than one file is given, any file after \""sfmt"\" will be skipped\n", src_file);
        else src_file = argv[c];
    }

    FILE *file;
    if (!src_file) {
        fprintf(stderr, ASM_NAME": error: no input file\n");
        error = true;
    } else {
        if (!(file = fopen(src_file, s("r")))) {
            fprintf(stderr, ASM_NAME": error: couldn't open \""sfmt"\"\n", src_file);
            error = true;
        }
    }

    if (error) {
        fprintf(stderr, ASM_NAME": assembly terminated\n");
        if (file) fclose(file);
        return 1;
    }

    symbols = (Symbol*)malloc(symbol_alloc * sizeof(Symbol));
    labels = (Label*)malloc(label_alloc * sizeof(Label));
    bin = (byte*)malloc(bin_alloc);
    
    while (fgets(line, 256, file)) {
        nl = true;
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

    free(symbols);
    free(labels);

    if (error || asm_error) {
        fprintf(stderr, ASM_NAME": assembly terminated\n");
        free(bin);
        return 1;
    } else {
        if (!(file = fopen(out_file, s("wb")))) return 1;
        
        fwrite(bin, 1, bin_count, file);
        printf(ASM_NAME": %d bytes written\n", bin_count);
    
        free(bin);

        fclose(file);
    }

    return 0;
}