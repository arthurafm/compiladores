/*
Programa principal de impressão de tokens.
Este arquivo será posteriormente substituído.
*/
#include <stdio.h>
#define _(s) #s // https://gcc.gnu.org/onlinedocs/gcc-12.2.0/cpp/Stringizing.html
#include "tokens.h"
#define RET_SUCESSO 0
#define RET_TKNERRO 1
#define RET_DESCONH 2

extern int yylex(void);
extern int yylex_destroy(void);

extern FILE *yyin;
extern char *yytext;
extern int get_line_number (void);

/* protótipos deste módulo - as implementações estão após a main */
void print_token_normal (char *token);
void print_token_especial (int token);
int print_token (int token);

int main (int argc, char **argv) {
  int token = 0, retorno = 0;
  while ((token = yylex()) && retorno == 0) {
    retorno = print_token(token);
  }
  yylex_destroy();
  return retorno;
}

void print_nome(char *token) {
  printf("%d %s [%s]\n", get_line_number(), token, yytext);
}
void print_nome2(int token) {
  printf("%d TK_ESPECIAL [%c]\n", get_line_number(), token);
}
/* A função retorna RET_SUCESSO se o token é conhecido. Caso contrário:
   - retorna RET_TKNERRO se o token é de erro
   - retorna RET_DESCONH se o token é desconhecido */
int print_token(int token) {
  switch (token){
    case '-':
    case '!':
    case '*':
    case '/':
    case '%':
    case '+':
    case '<':
    case '>':
    case '{':
    case '}':
    case '(':
    case ')':
    case '=':
    case ',':
    case ';':              print_nome2 (token);               break;
    case TK_PR_INT:        print_nome(_(TK_PR_INT));          break;
    case TK_PR_FLOAT:      print_nome(_(TK_PR_FLOAT));        break;
    case TK_PR_BOOL:       print_nome (_(TK_PR_BOOL));        break;
    case TK_PR_IF:         print_nome (_(TK_PR_IF));          break;
    case TK_PR_ELSE:       print_nome (_(TK_PR_ELSE));        break;
    case TK_PR_WHILE:      print_nome (_(TK_PR_WHILE));       break;
    case TK_PR_RETURN:     print_nome (_(TK_PR_RETURN));      break;
    case TK_OC_LE:         print_nome (_(TK_OC_LE));          break;
    case TK_OC_GE:         print_nome (_(TK_OC_GE));          break;
    case TK_OC_EQ:         print_nome (_(TK_OC_EQ));          break;
    case TK_OC_NE:         print_nome (_(TK_OC_NE));          break;
    case TK_OC_AND:        print_nome (_(TK_OC_AND));         break;
    case TK_OC_OR:         print_nome (_(TK_OC_OR));          break;
    case TK_LIT_INT:       print_nome (_(TK_LIT_INT));        break;
    case TK_LIT_FLOAT:     print_nome (_(TK_LIT_FLOAT));      break;
    case TK_LIT_FALSE:     print_nome (_(TK_LIT_FALSE));      break;
    case TK_LIT_TRUE:      print_nome (_(TK_LIT_TRUE));       break;
    case TK_IDENTIFICADOR: print_nome (_(TK_IDENTIFICADOR));  break;
    case TK_ERRO:          print_nome (_(TK_ERRO)); return RET_TKNERRO; break;
    default: printf ("<Token inválido com o código %d>\n", token); return RET_DESCONH; break;
  }
  return RET_SUCESSO;
}
