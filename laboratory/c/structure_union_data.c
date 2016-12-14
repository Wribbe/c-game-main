#include <stdlib.h>
#include <stdio.h>

struct TypeA {
    int a;
    int b;
    int c;
    int d;
    int e;
} TypeA;

struct TypeB {
    float a;
    float b;
    float c;
} TypeB;

union wrapper_type {
    struct TypeA a;
    struct TypeB b;
};

struct Wrapper {
    void (*mod_function)(union wrapper_type);
    union wrapper_type type;
};

void type_a_func(union wrapper_type * type)
{
    struct TypeA * pointer = (struct TypeA * )&type->a;
    printf("The current ints are: %d %d %d %d %d\n", pointer->a, pointer->b, pointer->c, pointer->d, pointer->e);
}

void type_b_func(union wrapper_type * type)
{
    struct TypeB * pointer = (struct TypeB * )&type->b;
    printf("The current floats are: %f %f %f\n", pointer->a, pointer->b, pointer->c);
}

void wrapper_consumer(struct Wrapper * wrapper)
{
    void (*mod_function)(void * data) = wrapper->mod_function;
    mod_function(&wrapper->type);
}

int main(void)
{
    struct TypeB typeb = {
        1.0f,
        2.0f,
        3.0f,
    };

    struct Wrapper wrap = {
        type_b_func,
        .type.b = typeb,
    };

    wrapper_consumer(&wrap);

    struct Wrapper wrap2 = {
        type_a_func,
        .type.a = {3, 4, 5, 6, 7},
    };

    wrapper_consumer(&wrap2);
}
