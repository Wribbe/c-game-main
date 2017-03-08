#include <stdlib.h>
#include <stdio.h>

struct TypeA {
    int (*function)(int a, int b, int c, int d, int e);
    int a;
    int b;
    int c;
    int d;
    int e;
} TypeA;

struct TypeB {
    void (*function)(struct TypeB * pointer);
    float a;
    float b;
    float c;
} TypeB;

union wrapper_type {
    struct TypeA a;
    struct TypeB b;
};

struct wrapper {
    union wrapper_type type;
};

int type_a_func(int a, int b, int c, int d, int e)
{
    int max_int = 0;
    int ints[] = {a,b,c,d,e};
    for (size_t i=0; i<sizeof(ints); i++) {
        if (ints[i] > max_int) {
            max_int = ints[i];
        }
    }
    return max_int;
}

void type_b_func(struct TypeB * pointer)
{
    printf("The current floats are: %f %f %f\n", pointer->a, pointer->b, pointer->c);
}

void wrapper_consumer(struct wrapper * list, size_t size)
{
    for (size_t i=0; i<size; i++) {
        struct wrapper * current = &list[i];
        if (current->type.a.function == type_a_func) {
            printf("It was A!\n");
        } else if (current->type.b.function == type_b_func) {
            printf("It was B!\n");
        }
    }
}

int main(void)
{
    struct wrapper wrapper_list[] = {
        {.type.a={type_a_func, 1,2,3,4,5}},
        {.type.b={type_b_func, 3.0f, 5.0f, 9.0f}},
        {.type.a={type_a_func, 11,21,31,41,51}},
    };

    wrapper_consumer(wrapper_list, sizeof(wrapper_list));
}
