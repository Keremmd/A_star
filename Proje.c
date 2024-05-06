#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define KEYWORD 1
#define IDENTIFIER 2
#define END_OF_LINE 3
#define OPERATOR 4
#define INT_CONST 5
#define STRING_CONST 6 
#define COMMA 7 
#define QUOTE '"'

char *keywords[] = {"int", "text", "is", "loop", "times", "read", "write", "newLine"};
int num_keywords = sizeof(keywords) / sizeof(keywords[0]);

char *delimiters[] = {" ", ",", "(", ")", "\n", ".", ":", "{", "}"};
int num_delimiters = sizeof(delimiters) / sizeof(delimiters[0]);

char *operators[] = {"+", "/", "-", "*"};
int num_operators = sizeof(operators) / sizeof(operators[0]);

//Fonksiyonlar
int isKeyword(char *token);
int isDelimiter(char ch);
int isOperator(char ch);
int isIntegerConstant(char *token);
int isStringConstant(char *token);
int isStringCharacter(char ch, int *string_length);
int isIdentifierLengthValid(char *token);

int main() {
    FILE *input_file, *output_file;
    char ch, token[50];
    int index = 0;
    int inString = 0;
    int string_length = 0;


    //code.sta dosyasının açılmasını sağlayan kod.
    input_file = fopen("code.sta", "r");
    if (input_file == NULL) {
        printf("Hata: code.sta dosyasi acilamadi!\n");
        exit(1);
    }

    //code.lex dosyasının açılmasını sağlayan kod.
    output_file = fopen("code.lex", "w");
    if (output_file == NULL) {
        printf("Hata: code.lex dosyasi acilamadi!\n");
        exit(1);
    }

    // Karakterleri dosya kapanasıya kadar okuyan döngü.
    while ((ch = fgetc(input_file)) != EOF) {
        if (ch == QUOTE) {
            if (!inString) {
                inString = 1;
                fprintf(output_file, "String(\"");
            } else {
                inString = 0;
                fprintf(output_file, "\")\n");
            }
            string_length = 0;
        } else if (inString && isStringCharacter(ch, &string_length)) {
            fprintf(output_file, "%c", ch);
        } else if (isDelimiter(ch) && !inString) {
            if (index > 0) {
                token[index] = '\0';
                if (isKeyword(token)) {
                    fprintf(output_file, "Keyword(%s)\n", token);
                } else if (isIntegerConstant(token)) {
                    fprintf(output_file, "IntConst(%s)\n", token);
                } else {
                    if(isIdentifierLengthValid(token)) {
                        fprintf(output_file, "Identifier(%s)\n", token);
                    }
                }
                memset(token, 0, sizeof(token));
                index = 0;
            }
            if (ch == '.') {
                fprintf(output_file, "EndOfLine\n");    //. gördüğünde EndOfLine yazdıran kod. 
            }
            if (ch == '{') {
                fprintf(output_file, "LeftCurlyBracket\n");     // { gördüğünde LeftCurlyBracket yazdıran kod.
            }
            if (ch == '}') {
                fprintf(output_file, "RightCurlyBracket\n");    // } gördüğünde RightCurlyBracket yazdıran kod.
            }
            if (ch == ',') { // Virgül durumunu ekle
                fprintf(output_file, "Comma\n");
            }
        // Operatörü ve operatörün string içinde olup olmadığı algılayan kod parçası.
        } else if (isOperator(ch) && !inString) {
            if (index > 0) {
                token[index] = '\0';
                fprintf(output_file, "Identifier(%s)\n", token);
                memset(token, 0, sizeof(token));
                index = 0;
            }
            // Yorum satırını algılayan kod parçası.
            if (ch == '/') {
                char next_ch = fgetc(input_file);
                if (next_ch == '*') {
                    int comment_open = 1;
                    while ((ch = fgetc(input_file)) != EOF) {
                        if (ch == '*' && (next_ch = fgetc(input_file)) == '/') {
                            comment_open = 0;
                            break;
                        }
                    }
                    if (comment_open) {
                        printf("Hata: Yorumun sonu bulunamadi!\n");
                        exit(1);
                    }
                    continue;
                } else {
                    fprintf(output_file, "Operator(/)\n");
                    ungetc(next_ch, input_file);
                }
            } else {
                fprintf(output_file, "Operator(%c)\n", ch);
            }
        } else {
            token[index++] = ch;
        }
    }

    fclose(input_file);
    fclose(output_file);

    printf("Tokenler basariyla olusturuldu: code.sta -> code.lex\n");

    return 0;
}

//Bu kod parçası tokenin anahtar kelime olup olmadığını kontrol eder.
int isKeyword(char *token) {
    int i;
    for (i = 0; i < num_keywords; i++) {
        if (strcmp(token, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

//Bu kod parçası karakterin ayırıcı olup olmadığını kontrol eder.
int isDelimiter(char ch) {
    int i;
    for (i = 0; i < num_delimiters; i++) {
        if (ch == delimiters[i][0]) {
            return 1;
        }
    }
    return 0;
}

//Bu kod parçası verilen karakterin operatör olup olmadığını kontrol eder.
int isOperator(char ch) {
    int i;
    for (i = 0; i < num_operators; i++) {
        if (ch == operators[i][0]) {
            return 1;
        }
    }
    if (ch == '/') {
        return 1;
    }
    return 0;
}

//Bu kod parçası verilen karakterin bir tamsayı sabiti olup olmadığını kontrol eder.
int isIntegerConstant(char *token) {
    if (!isdigit(token[0]) && token[0] != '+' && token[0] != '-') {
        return 0;
    }

    for (int i = 1; token[i] != '\0'; i++) {
        if (!isdigit(token[i])) {
            return 0;
        }
    }

    if (strlen(token) > 8) {
        printf("Hata: Tamsayi sabiti 8 haneliden uzun olamaz!\n");
        exit(1);
    }

    return 1;
}

//Bu kod parçası string uzunluğunu kontrol eder.
int isStringConstant(char *token) {
    if (strlen(token) > 258) {
        printf("Hata: String sabiti 258 karakterden uzun olamaz!\n");
        return 0;
    }
    return 1;
}

//Bu kod parçası identifier uzunluğunu kontrol eder.
int isIdentifierLengthValid(char *token) {
    if (!isalpha(token[0])) {
        printf("Hata: Identifier alfabetik bir karakterle baslamalidir!\n");
        exit(1);
    }
    if (strlen(token) > 10) {
        printf("Hata: Identifier 10 karakterden uzun olamaz!\n");
        exit(1);
    }
    return 1;
}

//Bu kod parçası string uzunluğunu kontrol eder.
int isStringCharacter(char ch, int *string_length) {
    if (ch == QUOTE) {
        *string_length = 0;
        return 0;
    } else if (isDelimiter(ch) || isOperator(ch)) {
        if (*string_length >= 256) {
            printf("Hata: String ifadeleri maksimum 256 karakter olabilir!\n");
            exit(1);
        }
        (*string_length)++;
        return 1;
    } else if (*string_length < 256) {
        (*string_length)++;
        return 1;
    } else {
        printf("Hata: String ifadeleri maksimum 256 karakter olabilir!\n");
        exit(1);
    }
}
