#include <stdlib.h>
#include <stdio.h>

#include <time.h>

typedef enum element_type {
    BLOCKING,
    PASSTHROUGH,
} element_type;

typedef struct List_Element {
    size_t value;
    int life;
    element_type type;
    struct List_Element * next;
    struct List_Element * sub_list;
    struct List_Element * last_in_sub_list;
} List_Element;

List_Element * global_list = NULL;
List_Element * last = NULL;

int get_value(size_t max) {
    return rand()%(int)max;
}

size_t global_value = 0;

void create_element(size_t value, List_Element ** append_to, List_Element ** last, int life) {

    // Create the element.
    List_Element * current_element = malloc(sizeof(List_Element));

    // Populate.
    current_element->value = value;
    current_element->sub_list = NULL;
    current_element->next = NULL;
    current_element->last_in_sub_list = NULL;
    if (life > 0) { // Set a life value. DEBUG
        current_element->life = life;
    } else { // Random life.
        current_element->life = get_value(10)+1;
    }
    int blocking = get_value(5);
    if (blocking < 3) {
        current_element->type = PASSTHROUGH;
    } else {
        current_element->type = BLOCKING;
    }

    if (*append_to == NULL) {
        *append_to = current_element;
        *last = current_element;
    } else {
        (*last)->next = current_element;
        *last = current_element;
    }
}

void print_indent(int indent)
{
    for (int i=0; i<indent; i++) {
        printf(" ");
    }
}

void print_element(List_Element * element, int indent)
{
    print_indent(indent);
    const char * type = "";
    if (element->type == BLOCKING) {
        type = "BLOCKING";
    } else {
        type = "PASSTHROUGH";
    }
    printf("value: %zu life: %d type: %s\n", element->value, element->life, type);
}


void process_values(List_Element ** list_for_processing, List_Element ** last_element, int indent, int print, int subelement)
{

    // Dereference to the value and then take the address.
    List_Element * current_element = &**list_for_processing;
    List_Element * prev = NULL;
    List_Element * temp = NULL;

    // Pointers for saving values for when switching sub-elements.
    List_Element * current_next = NULL;
    List_Element * current_last_sub = NULL;

    // Iterate as long as the current element is not NULL.
    while (current_element != NULL) {

        // Save current as temp.
        temp = current_element;

        // This is the "work" done by the thread.
        if (print) {
            print_element(current_element, indent);
        }

        global_value += 1;

        // Check for sub-elements.
        int has_sub_elements = current_element->sub_list != NULL;

        if (temp->type != BLOCKING) { // Never process sub-elements when BLOCKING.
            // Process sub-values.
            if (has_sub_elements) {
                process_values(&current_element->sub_list, &current_element->last_in_sub_list, indent+4, print, subelement+1);
            }
        }

        // Decrease the life of current element.
        current_element->life -= 1;

        // Is the element dead?
        int current_dead = current_element->life == 0;

        if (current_dead) {

            // Update the sub_element status since sub-elements might have
            // changed dut to relikning.
            has_sub_elements = current_element->sub_list != NULL;

            // If there are sub-elements, bubble them up to the parent
            // position instead of discarding the position that was taken by
            // the parent node. This node should not be run direly, since the
            // parent node already has processed all the children nodes for
            // this time around.
            if(has_sub_elements) {

                // Save the current_elements next pointer.
                current_next = current_element->next;
                // Save the current_element last_sub pointer.
                current_last_sub = current_element->last_in_sub_list;

                // Set the current pointer to the first sub-element.
                current_element = current_element->sub_list;
                // Move the other sub-list items from next to sub-list.
                current_element->sub_list = current_element->next;

                // Restore the old next pointer.
                current_element->next = current_next;
                // Restore the old last in sub-list pointer.
                current_element->last_in_sub_list = current_last_sub;

                // Check where we are in the list.
                if (prev == NULL) { // First element in the list, re-write anchor.
                    *list_for_processing = current_element;
                } else { // Not the first, re-link previous.
                    prev->next = current_element;
                }

                // Set the previous pointer to us.
                prev = current_element;

                // Advance the pointer, this sub-node should already have been
                // processed.
                current_element = current_element->next;


                // Free the dead parent node.
                free(temp);

                // Continue with the iteration.
                continue;

            } else { // Had no sub-elements.

                // De-link the dead node and de-allocate it.

                if (prev == NULL) { // First element.

                    // Set the global pointer to the current next element.
                    // Free the current element and continue to avoid advancing the
                    // pointer.

                    *list_for_processing=current_element->next;
                    current_element = current_element->next;
                    free(temp);

                    // Continue iteration.
                    continue;

                } else { // Middle or last.

                    if (current_element->next == NULL) { // Last.
                        // Set the previous pointer next to NULL.
                        prev->next = NULL;
                        // Re-write the last pointer to the previous element.
                        *last_element = prev;
                    } else { // Middle.
                        // Link around the current.
                        prev->next = current_element->next;
                    }

                    // Set prev as current element for the correct advancement.
                    current_element = prev;

                }

                // Free the dead current element.
                free(temp);

            }
        }

        // Check if we are a sub-level element, return instead of continuing.
        if (current_element->type == BLOCKING && subelement != 0) {
            return;
        }

        // Advance pointers.
        prev = current_element;
        current_element = current_element->next;
    }
}

int main(void) {

    srand(time(NULL));

    size_t max_num = 100;
    size_t max_sub_elements = 750;

    int print_stating_elements = 0;
    int print_process = 0;

    // Create all elements.
    for (size_t i=0; i<max_num; i++) {
        create_element(i+1, &global_list, &last, 1);
        // Create sub elements.
        int num_sub = get_value(max_sub_elements)+2;
        for (int j=0; j<num_sub; j++) {
            create_element((1+i)*10+j, &last->sub_list, &last->last_in_sub_list, 0);
        }
    }

    if (print_stating_elements) {

        List_Element * pointer = global_list;
        List_Element * sub_pointer = NULL;

        while (pointer != NULL) {
            print_element(pointer, 0);
          sub_pointer = pointer->sub_list;
            while(sub_pointer != NULL) {
                print_element(sub_pointer, 4);
                sub_pointer = sub_pointer->next;
            }
            pointer = pointer->next;
        }

        printf("\n\n\n");
    }

    while (global_list != NULL) {
        process_values(&global_list, &last, 0, print_process, 0);
        if (print_process) {
            printf("\n\n### NEW ROUND ### \n\n\n");
        }
    }

    printf("Resulting global value: %zu\n", global_value);
}
