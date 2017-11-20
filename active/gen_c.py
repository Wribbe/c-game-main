#!/usr/bin/env python3

GLOBAL_INDENT_STEP = 2

def format_imports(list_libs):

    def wrapper(lib):
        if not lib.startswith('<'):
            return "#include \"{}\"".format(lib)
        else:
            return "#include {}".format(lib)

    list_formatted = []
    for lib in list_libs:
        if lib.strip():
            list_formatted.append(wrapper(lib))
        else:
            list_formatted.append("")
    return '\n'.join(list_formatted)

def printf(string_format, *variables):
    buffer_returnstring = ["\"{}\\n\"".format(string_format)]
    new_vars = []
    for var in variables:
        if type(var) == str:
            new_vars.append("\"{}\"".format(var))
        else:
            new_vars.append(str(var))
    buffer_returnstring += new_vars
    return "printf({});".format(','.join(buffer_returnstring))

function_key_body = "body"
function_key_name = "name"
function_key_return_type = "return_type"
function_key_arguments = "arguments"
function_key_indent = "current_indentation"
function_key_open_contexts = "open_contexts"
function_key_defined_varaibles = "defined_variables"

def function_get(type_return, string_name, arguments=[]):

    dict_function = {
            function_key_body : [],
            function_key_name : string_name,
            function_key_return_type: type_return,
            function_key_arguments : arguments,
            function_key_indent : 2,
            function_key_open_contexts : [],
            function_key_defined_varaibles : set(),
    }

    return dict_function

def function_body_add(dict_function, string):
    function_body = dict_function[function_key_body]
    indent = dict_function[function_key_indent]
    function_body.append("{}{}".format(' '*indent, string))

def function_body_append_to_last(dict_function, string):
    function_body = dict_function[function_key_body]
    if not function_body: # No body currently, use add instead.
        function_body_add(dict_function, string)
    else: # Modify the last string in body.
        function_body[-1] += string

def function_glfw_setup():

    function = function_get("GLFWwindow *", "setup_glfw")

    glfw_setup_code = \
    """
    GLFWwindow * window;

    if (!glfwInit()) {
        fprintf(stderr, "Could not initialize glfw, aborting.\\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "HELLO WORLD", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Could not create window, aborting.\\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    /* Make window context current one. */
    glfwMakeContextCurrent(window);

    if (gl3wInit()) {
        fprintf(stderr, "Could not initialize gl3w, aborting.\\n");
        exit(EXIT_FAILURE);
    }

    if (!gl3wIsSupported(3, 3)) {
        fprintf(stderr, "Profile 3.3 not supported, aborting.\\n");
        exit(EXIT_FAILURE);
    }

    printf("OpenGL %s, GLSL %s\\n", glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION));

    return window;
    """
    function_body_add(function, glfw_setup_code)
    return function

def function_to_string(dict_function):

    buffer_string_return = []

    # Get return type.
    return_type = dict_function[function_key_return_type]
    buffer_string_return.append(return_type)

    # Construct function definition.
    function_definiton = ""
    function_definiton += dict_function[function_key_name] + '('
    arguments = dict_function[function_key_arguments]
    if not arguments:
        function_definiton += "void"
    else:
        function_definiton += ', '.join(arguments)
    function_definiton += ')'
    # Append function definition.
    buffer_string_return.append(function_definiton)

    # Open function scope.
    buffer_string_return.append("{")
    # Add body.
    function_body = '\n'.join(dict_function[function_key_body])
    buffer_string_return.append(function_body)
    # Close function scope.
    buffer_string_return.append("}")

    return '\n'.join(buffer_string_return)

def function_increment_indent(dict_function):
    dict_function[function_key_indent] += GLOBAL_INDENT_STEP

def function_decrement_indent(dict_function):
    dict_function[function_key_indent] -= GLOBAL_INDENT_STEP
    if dict_function[function_key_indent] < 0:
        dict_function[function_key_indent] = 0

def function_context_add(dict_function, context):
    dict_function[function_key_open_contexts].append(context)

def function_context_pop(dict_function):
    context = dict_function[function_key_open_contexts].pop()
    return context

def function_context_peek(dict_function, index=-1):
    list_context = dict_function[function_key_open_contexts]
    if not list_context:
        return ""
    return list_context[index]

def function_context_open(dict_function, contexct_type, condition):

    # Build context line.
    context_line = "{} ({}) {{".format(contexct_type, condition)
    function_context_add(dict_function, contexct_type)

    # Add the context line.
    if contexct_type in ["else", "else if"]:
        function_context_close(dict_function)
        function_body_append_to_last(dict_function, " "+context_line)
    else:
        function_body_add(dict_function, context_line)

    # Additional indent increment for context body.
    function_increment_indent(dict_function)

def function_context_close(dict_function):
    context_previous = function_context_pop(dict_function)
    function_decrement_indent(dict_function)
    function_body_add(dict_function, "}")
    return context_previous

assign_key_name_variable = "assignment_variable_name"
assign_key_assign_string = "assignment_string"
assign_key_assign_type = "assignment_type"

def assign(name_variable, string_value, string_type, *arguments):

    if string_value.endswith(")"): # Value is a function.
        string_value = string_value[:-1] # Cut last ')'.
        string_assign_value = "{}{}".format(string_value, ', '.join(arguments))
        string_assign_value += ")"
    else:
        string_assign_value = string_value

    dict_assignment = {
            assign_key_name_variable : name_variable,
            assign_key_assign_string : string_assign_value,
            assign_key_assign_type : string_type,
    }

    return dict_assignment

def function_add_assignment(dict_function, dict_assignment):
    name_variable = dict_assignment[assign_key_name_variable]
    string_value = dict_assignment[assign_key_assign_string]
    function_variable_set = dict_function[function_key_defined_varaibles]
    if name_variable not in function_variable_set:
        function_variable_set.add(name_variable)
        name_variable = "{} {}".format(dict_assignment[assign_key_assign_type],
                name_variable)
    function_body_add(dict_function, "{} = {};".format(name_variable, string_value))

def function_get_name(dict_function):
    return "{}()".format(dict_function[function_key_name])

output_key_includes = "includes"
output_key_definitions = "definitions"
output_key_functions = "functions"

def output_get():
    output = {
        output_key_includes : [],
        output_key_definitions : {},
        output_key_functions : [],
    }
    return output

def output_add_includes(output, list_includes):
    if type(list_includes) == str:
        list_includes = [list_includes]
    elif type(list_includes) != list:
        list_includes = [list_includes]

    output_list_includes = output[output_key_includes]

    for include in list_includes:
        if not include.strip():
            output_list_includes.append("")
            continue

        if not include.startswith("<"):
            include = "\"{}\"".format(include)
        include = "#include {}".format(include)
        output_list_includes.append(include)

def output_add_function(output, dict_function):
    output[output_key_functions].append(dict_function)

def output_add_definitions(output, dict_definitions):
    output[output_key_definitions].update(dict_definitions)

def output_to_string(output):

    output_buffer = []

    def sep():
        output_buffer.append("")

    # Print inclusions.
    for include in output[output_key_includes]:
        output_buffer.append(include)
    sep()

    # Print definitions.
    for definition_name, value in output[output_key_definitions].items():
        output_buffer.append("#define {} {}".format(
            definition_name.upper(),
            value))
    sep()

    # Print all functions.
    for function in output[output_key_functions]:
        output_buffer.append(function_to_string(function))
        sep()

    final_string = '\n'.join(output_buffer)
    return final_string

def output_print(output):
    print(output_to_string(output))

def main():

    output_main = output_get()

    includes = [
        "<math.h>",
        "<stdio.h>",
        "<stdlib.h>",
        "<string.h>",
        "",
        "gl3w.h",
        "<GLFW/glfw3.h>",
        "",
        "linmath.h",
    ]

    definitions = {
        "window_width" : 1440,
        "window_height" : 900,
    }

    output_add_includes(output_main, includes)
    output_add_definitions(output_main, definitions)

    output_add_function(output_main, function_glfw_setup())

    main = function_get("int", "main")

    name_window = "window"
    assignment_window = assign(name_window,
            function_get_name(function_glfw_setup()), "GLFWwindow *")
    function_add_assignment(main, assignment_window)

    function_context_open(main, "while",
            "!glfwWindowShouldClose({})".format(name_window))

    function_body_add(main, "glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);")
    function_body_add(main, "glfwPollEvents();")
    function_body_add(main, "glfwSwapBuffers({});".format(name_window))

    function_context_close(main);
    function_body_add(main, "glfwTerminate();")

    output_add_function(output_main, main)

    output_print(output_main)

if __name__ == "__main__":
        main()
