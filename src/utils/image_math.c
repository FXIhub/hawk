#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <matheval.h>
#include <ctype.h>
#include "spimage.h"

#define STACK_MAX 100

/* All operators are lowercase and all variables are uppercase! 
   Variables names start with A for the first given file B for the second
   and so on until Z, AA, AB, AC ...
   A typical expression would be (A - B) / abs(C).
*/

struct _TokenStack;

typedef enum{Prefix, Infix, Postfix} OperatorPosition;
typedef enum{LeftAssociative,RightAssociative,NonAssociative}OperatorAssociativity;
typedef enum{Operand=1,Comma,LeftParenthesis,RightParenthesis,Addition,UnaryPlus,Subtraction,
	     UnaryMinus,Multiplication,Division,Exponentiation,AbsoluteValue,
	     FourierTransform,InverseFourierTransform,Integrate,RealPart,ImaginaryPart,
	     ImaginaryUnit,ComplexArgument,Exponential,Logarythm,Minimum,Maximum,ComplexConjugate}TokenName;
typedef enum{TokenImage,TokenScalar,TokenOperator,TokenComma}TokenType;

typedef struct{
  /* function pointer to the function that executes the operator */
  void (*op)( struct _TokenStack *);
  /* number of operands that the operator applies to */
  int n_operands;
  /* higher numbers take precedence over lower numbers */
  int precedence; 
  OperatorPosition position;
  OperatorAssociativity associativity;
  /* string identifier(s)
     NULL terminated list of strings 
  */
  char * identifier[10];
  TokenName name;
}Operator;

/* All Elements are either an operator or an Image*/
typedef struct{
  TokenType type;
  Operator * operator;
  Image * image;
  Complex scalar;
}Token;

typedef struct _TokenStack{
  Token * stack[STACK_MAX];
  int top;
}TokenStack;



static char * sp_strdup(char * str){
  char * p = malloc(sizeof(char)*(strlen(str)+1));
  strcpy(p,str);
  return p;
}

TokenStack * token_stack_init(){
  TokenStack * ret = malloc(sizeof(TokenStack));
  ret->top = 0;
  return ret;
}

Token * token_stack_pop(TokenStack * stack){
  return stack->stack[--stack->top];
}

Token * token_stack_top(TokenStack * stack){
  return stack->stack[stack->top-1];
}

int token_stack_size(TokenStack * stack){
  return stack->top;
}

void token_stack_push(TokenStack * stack, Token * a){
  stack->stack[stack->top++] = a;
}

void token_stack_add(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  Token * b = token_stack_pop(stack);
  
  if(a->type == TokenImage && b->type == TokenImage){
    sp_image_add(a->image,b->image);
    sp_image_free(b->image);
    token_stack_push(stack,a);  
  }else if(a->type == TokenImage && b->type == TokenScalar){ 
    for(int i = 0;i<sp_image_size(a->image);i++){
      a->image->image->data[i] = sp_cadd(a->image->image->data[i],b->scalar);
    }
    token_stack_push(stack,a);  
  }else if(b->type == TokenImage && a->type == TokenScalar){
    for(int i = 0;i<sp_image_size(b->image);i++){
      b->image->image->data[i] = sp_cadd(b->image->image->data[i],a->scalar);
    }    
    token_stack_push(stack,b);  
  }else if(a->type == TokenScalar && b->type == TokenScalar){
    a->scalar = sp_cadd(a->scalar,b->scalar);
    token_stack_push(stack,a);
  }else{
    abort();
  }
}

void token_stack_mul(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  Token * b = token_stack_pop(stack);
  if(a->type == TokenImage && b->type == TokenImage){
    sp_image_mul_elements(a->image,b->image);
    sp_image_free(b->image);
    token_stack_push(stack,a);  
  }else if(a->type == TokenImage && b->type == TokenScalar){ 
    for(int i = 0;i<sp_image_size(a->image);i++){
      a->image->image->data[i] = sp_cmul(a->image->image->data[i],b->scalar);
    }
    token_stack_push(stack,a);  
  }else if(b->type == TokenImage && a->type == TokenScalar){
    for(int i = 0;i<sp_image_size(b->image);i++){
      b->image->image->data[i] = sp_cmul(b->image->image->data[i],a->scalar);
    }    
    token_stack_push(stack,b);  
  }else if(a->type == TokenScalar && b->type == TokenScalar){
    a->scalar = sp_cmul(a->scalar,b->scalar);
    token_stack_push(stack,a);
  }else{
    abort();
  }
}

void token_stack_div(TokenStack * stack){
  Token * b = token_stack_pop(stack);
  if(b->type == TokenImage){
    sp_image_invert(b->image);
  }else if(b->type == TokenScalar){
    if(sp_real(b->scalar)){
      sp_real(b->scalar) = 1.0/(sp_real(b->scalar));
    }
    if(sp_imag(b->scalar)){
      sp_imag(b->scalar) = 1.0/(sp_imag(b->scalar));
    }
  }else{
    abort();
  }
  token_stack_push(stack,b);  
  token_stack_mul(stack);
}

void token_stack_abs(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  if(a->type == TokenImage){
    sp_image_dephase(a->image);
  }else if(a->type == TokenScalar){
    sp_real(a->scalar) = sp_cabs(a->scalar);
    sp_imag(a->scalar) = 0;
  }else{
    abort();
  }
  token_stack_push(stack,a);  
}

void token_stack_conj(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  if(a->type == TokenImage){
    for(int i = 0;i<sp_image_size(a->image);i++){
      sp_imag(a->image->image->data[i]) = -sp_imag(a->image->image->data[i]);
    }
  }else if(a->type == TokenScalar){
    sp_imag(a->scalar) = -sp_imag(a->scalar);
  }else{
    abort();
  }
  token_stack_push(stack,a);  
}


void token_stack_sub(TokenStack * stack){
  Token * b = token_stack_pop(stack);
  if(b->type == TokenScalar){
    b->scalar = sp_cmul(b->scalar,sp_cinit(-1,0));
  }else if(b->type == TokenImage){
    sp_image_scale(b->image,-1);
  }else{
    abort();
  }
  token_stack_push(stack,b);  
  token_stack_add(stack);
}

void token_stack_unary_plus(TokenStack * stack){
  /**/
  stack = NULL;
  return;
}

void token_stack_comma(TokenStack * stack){
  /**/
  stack = NULL;
  return;
}

void token_stack_unary_minus(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  if(a->type == TokenImage){
    sp_image_scale(a->image,-1);
  }else if(a->type == TokenScalar){
    sp_real(a->scalar) *= -1;
    sp_imag(a->scalar) *= -1;
  }else{
    abort();
  }
  token_stack_push(stack,a);  
}

void token_stack_pow(TokenStack * stack){
  Token * b = token_stack_pop(stack);
  Token * a = token_stack_pop(stack);
  if(b->type != TokenScalar){
    fprintf(stderr,"Can only exponentiate to a scalar power!\n");
    abort();
  }
  if(a->type == TokenImage){
    for(int i = 0;i<sp_image_size(a->image);i++){
      a->image->image->data[i] = sp_cpow(a->image->image->data[i],b->scalar);
    }
  }else if(a->type == TokenScalar){
    a->scalar = sp_cpow(a->scalar,b->scalar);
  }
  token_stack_push(stack,a);  
}

void token_stack_exp(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  if(a->type == TokenImage){
    for(int i = 0;i<sp_image_size(a->image);i++){
      a->image->image->data[i] = sp_cexp(a->image->image->data[i]);
    }
  }else if(a->type == TokenScalar){
    a->scalar = sp_cexp(a->scalar);
  }
  token_stack_push(stack,a);  
}

void token_stack_log(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  if(a->type == TokenImage){
    for(int i = 0;i<sp_image_size(a->image);i++){
      a->image->image->data[i] = sp_clog(a->image->image->data[i]);
    }
  }else if(a->type == TokenScalar){
    a->scalar = sp_clog(a->scalar);
  }
  token_stack_push(stack,a);  
}

void token_stack_fft(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  if(a->type != TokenImage){
    fprintf(stderr,"Can only do fft on images!\n");
    abort();
  }
  Image * b = sp_image_fft(a->image);
  sp_image_free(a->image);
  a->image = b;
  token_stack_push(stack,a);  
}

void token_stack_ifft(TokenStack * stack){  
  Token * a = token_stack_pop(stack);
  if(a->type != TokenImage){
    fprintf(stderr,"Can only do ifft on images!\n");
    abort();
  }
  a->image->phased = 1;
  a->image->shifted = 1;
  Image * b = sp_image_ifft(a->image);
  sp_image_free(a->image);
  a->image = b;
  token_stack_push(stack,a);  
}

void token_stack_real(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  if(a->type == TokenScalar){
    sp_imag(a->scalar) = 0;
  }else{
    for(int i = 0;i<sp_image_size(a->image);i++){
      sp_imag(a->image->image->data[i]) = 0;
    }
  }
  token_stack_push(stack,a);  
}

void token_stack_arg(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  if(a->type == TokenScalar){
    sp_real(a->scalar) = sp_carg(a->scalar);
  }else{
    for(int i = 0;i<sp_image_size(a->image);i++){
      sp_real(a->image->image->data[i]) = sp_carg(a->image->image->data[i]);
      sp_imag(a->image->image->data[i]) = 0;
    }
  }
  token_stack_push(stack,a);  
}

void token_stack_imag(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  if(a->type == TokenScalar){
    sp_real(a->scalar) = 0;
  }else{
    for(int i = 0;i<sp_image_size(a->image);i++){
      sp_real(a->image->image->data[i]) = 0;
    }
  }
  token_stack_push(stack,a);  
}

void token_stack_imaginary_unit(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  if(a->type == TokenScalar){
    a->scalar = sp_cmul(a->scalar,sp_cinit(0,1));
  }else{
    for(int i = 0;i<sp_image_size(a->image);i++){
      a->image->image->data[i] = sp_cmul(a->image->image->data[i],sp_cinit(0,1));
    }  
  }
  token_stack_push(stack,a);  
}

void token_stack_integrate(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  if(a->type == TokenScalar){
    token_stack_push(stack,a);  
    return;
  }
  a->scalar = sp_image_integrate(a->image);
  sp_image_free(a->image);
  a->image = NULL;
  a->type = TokenScalar;
  token_stack_push(stack,a);  
}

void token_stack_min(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  Token * b = token_stack_pop(stack);
  
  if(a->type == TokenImage && b->type == TokenImage){
    for(int i = 0;i<sp_image_size(a->image);i++){
      sp_real(a->image->image->data[i]) = sp_min(sp_real(a->image->image->data[i]),sp_real(b->image->image->data[i]));
      sp_imag(a->image->image->data[i]) = sp_min(sp_imag(a->image->image->data[i]),sp_imag(b->image->image->data[i]));
    }
    token_stack_push(stack,a);  
  }else if(a->type == TokenImage && b->type == TokenScalar){ 
    for(int i = 0;i<sp_image_size(a->image);i++){
      sp_real(a->image->image->data[i]) = sp_min(sp_real(a->image->image->data[i]),sp_real(b->scalar));
      sp_imag(a->image->image->data[i]) = sp_min(sp_imag(a->image->image->data[i]),sp_imag(b->scalar));
    }
    token_stack_push(stack,a);  
  }else if(b->type == TokenImage && a->type == TokenScalar){
    for(int i = 0;i<sp_image_size(b->image);i++){
      sp_real(b->image->image->data[i]) = sp_min(sp_real(b->image->image->data[i]),sp_real(a->scalar));
      sp_imag(b->image->image->data[i]) = sp_min(sp_imag(b->image->image->data[i]),sp_imag(a->scalar));
    }    
    token_stack_push(stack,b);  
  }else if(a->type == TokenScalar && b->type == TokenScalar){
    sp_real(a->scalar) = sp_min(sp_real(a->scalar),sp_real(b->scalar));
    sp_imag(a->scalar) = sp_min(sp_imag(a->scalar),sp_imag(b->scalar));
    token_stack_push(stack,a);
  }else{
    abort();
  }
}

void token_stack_max(TokenStack * stack){
  Token * a = token_stack_pop(stack);
  Token * b = token_stack_pop(stack);
  
  if(a->type == TokenImage && b->type == TokenImage){
    for(int i = 0;i<sp_image_size(a->image);i++){
      sp_real(a->image->image->data[i]) = sp_max(sp_real(a->image->image->data[i]),sp_real(b->image->image->data[i]));
      sp_imag(a->image->image->data[i]) = sp_max(sp_imag(a->image->image->data[i]),sp_imag(b->image->image->data[i]));
    }
    token_stack_push(stack,a);  
  }else if(a->type == TokenImage && b->type == TokenScalar){ 
    for(int i = 0;i<sp_image_size(a->image);i++){
      sp_real(a->image->image->data[i]) = sp_max(sp_real(a->image->image->data[i]),sp_real(b->scalar));
      sp_imag(a->image->image->data[i]) = sp_max(sp_imag(a->image->image->data[i]),sp_imag(b->scalar));
    }
    token_stack_push(stack,a);  
  }else if(b->type == TokenImage && a->type == TokenScalar){
    for(int i = 0;i<sp_image_size(b->image);i++){
      sp_real(b->image->image->data[i]) = sp_max(sp_real(b->image->image->data[i]),sp_real(a->scalar));
      sp_imag(b->image->image->data[i]) = sp_max(sp_imag(b->image->image->data[i]),sp_imag(a->scalar));
    }    
    token_stack_push(stack,b);  
  }else if(a->type == TokenScalar && b->type == TokenScalar){
    sp_real(a->scalar) = sp_max(sp_real(a->scalar),sp_real(b->scalar));
    sp_imag(a->scalar) = sp_max(sp_imag(a->scalar),sp_imag(b->scalar));
    token_stack_push(stack,a);
  }else{
    abort();
  }
}


static Operator operator_table[100] = {
  {
    .op = token_stack_add,
    .n_operands = 2,
    .precedence = 1,
    .associativity = LeftAssociative,
    .position = Infix,
    .identifier = {"+",NULL},
    .name = Addition
  },
  {
    .op = token_stack_unary_plus,
    .n_operands = 1,
    .precedence = 3,
    .associativity = RightAssociative,
    .position = Infix,
    .identifier = {"+",NULL},
    .name = UnaryPlus
  },
  {
    .op = token_stack_sub,
    .n_operands = 2,
    .precedence = 1,
    .associativity = LeftAssociative,
    .position = Infix,
    .identifier = {"-",NULL},
    .name = Subtraction
  },
  {
    .op = token_stack_unary_minus,
    .n_operands = 1,
    .precedence = 3,
    .associativity = RightAssociative,
    .position = Infix,
    .identifier = {"-",NULL},
    .name = UnaryMinus
  },
  {
    .op = token_stack_mul,
    .n_operands = 2,
    .precedence = 2,
    .associativity = LeftAssociative,
    .position = Infix,
    .identifier = {"*",NULL},
    .name = Multiplication
  },
  {
    .op = token_stack_div,
    .n_operands = 2,
    .precedence = 2,
    .associativity = LeftAssociative,
    .position = Infix,
    .identifier = {"/",NULL},
    .name = Division
  },
  {
    .op = token_stack_conj,
    .n_operands = 1,
    .precedence = 5,
    .associativity = NonAssociative,
    .position = Prefix,
    .identifier = {"conj",NULL},
    .name = ComplexConjugate
  },
  {
    .op = token_stack_abs,
    .n_operands = 1,
    .precedence = 5,
    .associativity = NonAssociative,
    .position = Prefix,
    .identifier = {"abs",NULL},
    .name = AbsoluteValue
  },
  {
    .op = token_stack_pow,
    .n_operands = 1,
    .precedence = 4,
    .associativity = RightAssociative,
    .position = Infix,
    .identifier = {"^",NULL},
    .name = Exponentiation
  },
  {
    .op = token_stack_fft,
    .n_operands = 1,
    .precedence = 5,
    .associativity = NonAssociative,
    .position = Prefix,
    .identifier = {"fft",NULL},
    .name = FourierTransform
  },
  {
    .op = token_stack_ifft,
    .n_operands = 1,
    .precedence = 5,
    .associativity = NonAssociative,
    .position = Prefix,
    .identifier = {"ifft",NULL},
    .name = InverseFourierTransform
  },
  {
    .op = token_stack_integrate,
    .n_operands = 1,
    .precedence = 5,
    .associativity = NonAssociative,
    .position = Prefix,
    .identifier = {"sum",NULL},
    .name = Integrate
  },
  {
    .op = token_stack_arg,
    .n_operands = 1,
    .precedence = 5,
    .associativity = NonAssociative,
    .position = Postfix,
    .identifier = {"arg",NULL},
    .name = ComplexArgument
  },
  {
    .op = token_stack_exp,
    .n_operands = 1,
    .precedence = 5,
    .associativity = NonAssociative,
    .position = Prefix,
    .identifier = {"exp",NULL},
    .name = Exponential
  },  
  {
    .op = token_stack_log,
    .n_operands = 1,
    .precedence = 5,
    .associativity = NonAssociative,
    .position = Prefix,
    .identifier = {"log",NULL},
    .name = Logarythm
  },
  {
    .op = token_stack_min,
    .n_operands = 2,
    .precedence = 5,
    .associativity = NonAssociative,
    .position = Prefix,
    .identifier = {"min",NULL},
    .name = Minimum
  },
  {
    .op = token_stack_max,
    .n_operands = 2,
    .precedence = 5,
    .associativity = NonAssociative,
    .position = Prefix,
    .identifier = {"max",NULL},
    .name = Maximum
  },
  {
    .op = token_stack_real,
    .n_operands = 1,
    .precedence = 5,
    .associativity = NonAssociative,
    .position = Prefix,
    .identifier = {"re",NULL},
    .name = RealPart
  },
  {
    .op = token_stack_imag,
    .n_operands = 1,
    .precedence = 5,
    .associativity = NonAssociative,
    .position = Prefix,
    .identifier = {"im",NULL},
    .name = ImaginaryPart
  },
  {
    .op = token_stack_imaginary_unit,
    .n_operands = 1,
    .precedence = 3,
    .associativity = NonAssociative,
    .position = Postfix,
    .identifier = {"i",NULL},
    .name = ImaginaryUnit
  },
  {
    .op = token_stack_comma,
    .n_operands = 2,
    .precedence = 0,
    .associativity = NonAssociative,
    .position = Infix,
    .identifier = {",",NULL},
    .name = Comma
  },
  {
    .identifier = {NULL}
  }
};




static Token ** parse_tokens(Token ** infix);

static Token ** tokenize_string(char * input, Operator * op_table, Image ** image_list);

Token * evaluate_postfix(Token ** postfix){
  TokenStack * stack = token_stack_init();
  for(int i = 0;postfix[i];i++){
    if(postfix[i]->type == TokenOperator){
      if(token_stack_size(stack) < postfix[i]->operator->n_operands){
	fprintf(stderr,"%d operands expected but just %d in stack for operator %s!\n",postfix[i]->operator->n_operands,token_stack_size(stack), postfix[i]->operator->identifier[0]);
	abort();
      }
      postfix[i]->operator->op(stack);
    }else{
      token_stack_push(stack,postfix[i]);
    }
  }
  if(token_stack_size(stack) == 1){   
    return token_stack_pop(stack);
  }else{
    fprintf(stderr,"Stack contains %d members after evaluation. Expecting 1. Abort!\n",token_stack_size(stack));
    exit(1);
  }
  return NULL;
}

Token ** parse_tokens(Token ** infix){
  TokenStack * stack = token_stack_init();
  int infix_size = 0;
  for(;infix[infix_size];infix_size++);
  Token ** postfix = malloc(sizeof(Token *)*(infix_size+1));
  int postfix_index = 0;

  for(int i = 0;infix[i];i++){
    if(infix[i]->type != TokenOperator){
      postfix[postfix_index++] = infix[i];
    }else if(infix[i]->operator){

      while(token_stack_size(stack) && infix[i]->operator->precedence < token_stack_top(stack)->operator->precedence){
	postfix[postfix_index++] =  token_stack_pop(stack);
      }
      /* The rules for poping from the stack when the precedence is the same depend on the associativity */
      if(infix[i]->operator->associativity == LeftAssociative){
	while(token_stack_size(stack) && infix[i]->operator->precedence == token_stack_top(stack)->operator->precedence){
	  postfix[postfix_index++] =  token_stack_pop(stack);
	}
      }
      token_stack_push(stack,infix[i]);
    }else{
      abort();
    }
  }
  while(token_stack_size(stack)){
    postfix[postfix_index++] =  token_stack_pop(stack);
  }
  postfix[infix_size] = NULL;
  return postfix;
}

int index_from_variable_name(char * name){
  int index = 0;
  int base = 1;
  for(int i = strlen(name)-1;i>=0;i--){
    index += (name[i]-'A')*base;
    base *= ('Z'-'A')+1;
  }
  return index;
}

Token ** tokenize_string(char * input, Operator * op_table, Image ** image_list){
  Token pe[1000];
  int pe_used = 0;
  char * token_start = NULL;
  char * token_end = NULL;
  int image_list_size = 0;
  TokenName lastToken = 0;
  while(image_list[image_list_size]){
    image_list_size++;
  }
  int precedence_modifier = 0;
  for(int i = 0;i<(int)strlen(input);){
    if(isblank(input[i])){
      i++;
    }else if(input[i] == '('){
      precedence_modifier += 1000;
      lastToken = LeftParenthesis;
      i++;
    }else if(input[i] == ')'){
      precedence_modifier -= 1000;
      lastToken = RightParenthesis;
      i++;
    }else if(isdigit(input[i]) || input[i] == '.'){
      token_start = &(input[i]);
      double d = strtod(token_start,&token_end);
      if(token_end == NULL){
	/* number goes to the end of the string */
	i = strlen(input); 
      }else{
	/* otherwise fast forward i to the end of the number */
	while(&input[i] < token_end){
	  i++;
	}
      }
      pe[pe_used].operator = NULL;
      pe[pe_used].image = NULL;
      pe[pe_used].scalar = sp_cinit(d,0);
      pe[pe_used].type = TokenScalar;
      lastToken = Operand;
      pe_used++;      
    }else if(isupper(input[i])){
      token_start = &(input[i]);
      int j = i;
      while(isupper(input[j])){
	j++;
      }
      token_end = &(input[j]);
      /* temporarily truncate the string*/
      char tmp = input[j];
      input[j] = 0;
      int index = index_from_variable_name(token_start);
      if(index >= image_list_size){
	fprintf(stderr,"Variable %s has no matching file!\n",token_start);
	exit(1);
      }
      /* restore the string back */
      input[j] = tmp;
      /* fast forward i */
      i = j;
      Image * a = sp_image_duplicate(image_list[index],SP_COPY_ALL);
      pe[pe_used].operator = NULL;
      pe[pe_used].image = a;
      pe[pe_used].type = TokenImage;
      lastToken = Operand;
      pe_used++;            
    }else{
      /* Try to match any of the operands */
      token_start = &(input[i]);
      int operator_found_flag = 0;
      for(int j = 0;op_table[j].identifier[0];j++){
	for(int k = 0; op_table[j].identifier[k];k++){
	  if(strstr(token_start,op_table[j].identifier[k]) == token_start){
	    /* we found our operator */	    
	    if(op_table[j].name == Subtraction && (lastToken != Operand && lastToken != RightParenthesis && lastToken != ImaginaryUnit)){
	      /* this is actually a unary minus not a subtraction */
	      j++;
	    }
	    if(op_table[j].name == Addition && (lastToken != Operand && lastToken != RightParenthesis && lastToken != ImaginaryUnit)){
	      /* this is a unary plus not an addition */
	      j++;
	    }
	    if(op_table[j].name == ImaginaryUnit && (lastToken != Operand && lastToken != RightParenthesis && lastToken != ImaginaryUnit)){
	      pe[pe_used].operator = NULL;
	      pe[pe_used].image = NULL;
	      pe[pe_used].scalar = sp_cinit(0,1);
	      pe[pe_used].type = TokenScalar;
	      lastToken = Operand;
	      pe_used++;      
	      operator_found_flag = 1;
	      i++;
	      break;
	    }
	    pe[pe_used].operator = malloc(sizeof(Operator));
	    *pe[pe_used].operator = op_table[j];
	    /* Apply precedence modifier resulting from the parenthesis */
	    pe[pe_used].operator->precedence += precedence_modifier;
	    pe[pe_used].image = NULL;
	    pe[pe_used].type = TokenOperator;
	    operator_found_flag = 1;
	    lastToken = pe[pe_used].operator->name;
	    pe_used++;
	    /* fast forward i */
	    i += strlen(op_table[j].identifier[k]);
	    break;
	  }
	}
	if(operator_found_flag){
	  break;
	}
      }
      if(!operator_found_flag){
	fprintf(stderr,"Could not tokenize expression starting at %s!\n",token_start);
	exit(1);
      }
    }
  }
  Token ** ret = malloc(sizeof(Token *) * (pe_used+1));
  for(int i = 0;i<pe_used;i++){
    ret[i] = malloc(sizeof(Token));
    *ret[i] = pe[i];
  }
  ret[pe_used] = NULL;
  if(precedence_modifier != 0){
    fprintf(stderr,"Warning: Unbalanced parenthesis!\n");
  }
  return ret;
}

void check_image_size(Image ** image_list){
  if(image_list[0] == NULL){
    return;
  }
  int dim[3] = {sp_image_x(image_list[0]),sp_image_y(image_list[0]),sp_image_z(image_list[0])};
  for(int i = 1;image_list[i] != NULL;i++){
    if(sp_image_x(image_list[0]) != dim[0] || sp_image_y(image_list[0]) != dim[1] || sp_image_z(image_list[0]) != dim[2]){
      fprintf(stderr,"All images must have the same dimensions!\n Image number %d doesn't match the first image.\n",i+1);
      abort();
    }
  }
}

int main(int argc, char ** argv){
  int c;
  static char optstring[] = "e:o:h";
  char * expression = NULL;
  char * output_file = NULL;
  static char help_text[] = "\
    \n\
         Usage:\n\
    \n\
    image_math <-e expression> [-o output] [inputA] [inputB] ...\n\
    \n\
         Options description:\n\
    \n\
    -e: Expression to be applied to input\n\
    -o: Output file\n\
    -h: Print help text\n\
    \n\
    In the expression (specified by -e), the first input dataset\n\
    (from left to right) is  referred to as A, the second as B,\n\
    and so on.\n\
    Expressions use a C-like  infix  notation,  with  most  standard\n\
    operators  and  mathematical functions (+, sin, etc.) being sup‐\n\
    ported.\n\
";
  if(argc < 2){
    printf("Try image_math -h for help\n");
    return 0;
  }
  while(1){
    c = getopt(argc,argv,optstring);
    if(c == -1){
      break;
    }
    switch(c){
    case 'e':
      free(expression);
      expression = sp_strdup(optarg);
      break;
    case 'o':
      free(output_file);
      output_file = sp_strdup(optarg);
      break;
    case 'h':
      printf("%s",help_text);
      exit(0);
      break;
    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
    }
  }
  /* Now the rest of the arguments should be image files */
  Image ** image_list = malloc(sizeof(Image *)*(argc-optind));
  int image_list_size = 0;
  for(int i = optind;i<argc;i++){
    image_list[image_list_size++] = sp_image_read(argv[i],0);
  }
  image_list[image_list_size] = NULL;
  check_image_size(image_list);
  Token ** tokens = tokenize_string(expression,operator_table,image_list);
  Token ** postfix = parse_tokens(tokens);
  Token * out = evaluate_postfix(postfix);
  
  if(out->type == TokenScalar){
    Complex v = out->scalar;
    printf("%g + %g i\n",sp_real(v),sp_imag(v));
  }else if(out->type == TokenImage){
    if(output_file){
      printf("Writing output to %s\n",output_file);
      sp_image_write(out->image,output_file,0);
    }else{
      printf("Writing output to %s\n","output.h5");
      sp_image_write(out->image,"output.h5",0);
    }
  }else{
    fprintf(stderr,"Unexepected output type!\n");
    abort();
  }
  return 0;
}

typedef struct {
  char * output_filename;
  char ** input_filenames;
  int ninput_filenames;
  char * expression;
  void * evaluator;
  char ** variables;
  int nvariables;
  Image ** input_images;
  Image * output_image;
  int verbose;
}Options;

static void set_defaults(Options * opt);


#define CHECK(cond, msg) { if (!(cond)) { fprintf(stderr, "image_math error: %s\n", msg); exit(EXIT_FAILURE); } }

Options * parse_options(int argc, char ** argv){
  int c;
  static char help_text[] = 
    "    Options description:\n\
    \n\
    -e: Expression to be applied to input\n\
    -o: Output file\n\
    -v: Verbose\n\
    -h: Print help text\n\
    \n\
    In the expression (specified by -e), the first input dataset\n\
    (from left to right) is  referred to as d1, the second as d2,\n\
    and so on.\n\
    Expressions use a C-like  infix  notation,  with  most  standard\n\
    operators  and  mathematical functions (+, sin, etc.) being sup‐\n\
    ported.  This functionality is provided (and its features deter‐\n\
    mined) by GNU libmatheval.\n\
";
  static char optstring[] = "e:o:hv";
  Options * res = calloc(1,sizeof(Options));
  set_defaults(res);

  while(1){
    c = getopt(argc,argv,optstring);
    if(c == -1){
      break;
    }
    switch(c){
    case 'e':
      free(res->expression);
      res->expression = sp_strdup(optarg);
      break;
    case 'o':
      free(res->output_filename);
      res->output_filename = sp_strdup(optarg);
      break;
    case 'h':
      printf("%s",help_text);
      exit(0);
      break;
    case 'v':
      res->verbose = 1;
      break;
    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
    }
  }
  free(res->input_filenames);
  res->ninput_filenames = (argc-optind);
  res->input_filenames = malloc(sizeof(char *)*(argc-optind));
  for(int i = optind;i<argc;i++){
    res->input_filenames[i-optind] = sp_strdup(argv[i]);
  }
  return res;
}

void set_defaults(Options * opt){

  opt->output_filename = 0;
  opt->input_filenames = 0;
  opt->ninput_filenames = 0;
  opt->expression = 0;
  opt->evaluator = 0;
  opt->nvariables = 0;
  opt->variables = 0;
  opt->verbose = 0;
}
#if 0

int main(int argc, char ** argv){
  int i,j;
  int flag;
  Options * opts;
  double * vals;
  if(argc < 2){
    printf("Try image_math -h for help\n");
    return 0;
  }
  opts = malloc(sizeof(Options));
  set_defaults(opts);
  opts = parse_options(argc,argv);

  for (i = 0; i < strlen(opts->expression); ++i){
    if (opts->expression[i] == '\n'){
      opts->expression[i] = ' '; /* matheval chokes on newlines */
    }
  }

  CHECK(opts->evaluator = evaluator_create(opts->expression),"error parsing symbolic expression");
  evaluator_get_variables(opts->evaluator, &opts->variables, &opts->nvariables);

  /* Check variable names */
  flag = 0;
  for(i = 0;i<opts->nvariables;i++){
    if(opts->variables[i][0] != 'd'){
      flag = i;
      break;
    }
    if(strlen(opts->variables[i]) < 2){
      flag = i;
      break;
    }
    for(j = 1;j<strlen(opts->variables[i]);j++){
      if(!isdigit(opts->variables[i][j])){
      flag = i;
      break;
      }
    }
    if(flag){
      break;
    }
  }
  if(flag){
    fprintf(stderr, "image_math error: unrecognized variable \"%s\"\n",
	    opts->variables[flag]);
    exit(EXIT_FAILURE); 
  }

  opts->input_images = malloc(sizeof(Image  *)*opts->ninput_filenames);
  /* Read Input files */
  for(i = 0;i<opts->ninput_filenames;i++){
    opts->input_images[i] = sp_image_read(opts->input_filenames[i],0);
  }
  /* Create output file */
  opts->output_image = sp_image_duplicate(opts->input_images[0],0);
  
  if (opts->verbose) {
    char *buf = evaluator_get_string(opts->evaluator);
    printf("Evaluating expression: %s\n", buf);
  }
  vals = malloc(sizeof(double)*opts->nvariables);
  for(i = 0;i<sp_image_size(opts->input_images[0]);i++){
    for(j = 0;j<opts->ninput_filenames;j++){
      vals[j] = sp_real(opts->input_images[j]->image->data[i]);
    }
    sp_real(opts->output_image->image->data[i]) = evaluator_evaluate(opts->evaluator, opts->ninput_filenames, opts->variables, vals);
    for(j = 0;j<opts->ninput_filenames;j++){
      vals[j] = sp_imag(opts->input_images[j]->image->data[i]);
    }
    sp_imag(opts->output_image->image->data[i]) = evaluator_evaluate(opts->evaluator, opts->ninput_filenames, opts->variables, vals);
  }
  sp_image_write(opts->output_image,opts->output_filename,0);
  return 0;
}

#endif
