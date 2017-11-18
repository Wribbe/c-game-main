#!/usr/bin/env python3

prefix_local_lib = "!L"

def format_imports(list_libs):

    def wrapper(lib):
        if lib.startswith(prefix_local_lib):
            lib = lib.replace(prefix_local_lib, '')
            return "#include \"{}.h\"".format(lib)
        else:
            return "#include <{}.h>".format(lib)

    list_formatted = []
    for lib in list_libs:
        list_formatted.append(wrapper(lib))
    return '\n'.join(list_formatted)

def printf(string_format, variables=[]):
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

def function_get(type_return, string_name, arguments=[]):

    dict_function = {
            function_key_body : [],
            function_key_name : string_name,
            function_key_return_type: type_return,
            function_key_arguments : arguments,
            function_key_indent : 2,
    }

    return dict_function

def function_add(dict_function, string_function):
    function_body = dict_function[function_key_body]
    indent = dict_function[function_key_indent]
    function_body.append("{}{}".format(' '*indent, string_function))

def main():

    list_output = []

    def output_add(string):
        list_output.append(string)

    def output_sep():
        list_output.append("")

    def output_print():
        print('\n'.join(list_output))

    def output_dump_function(dict_function):
        # Add return type.
        output_add(dict_function[function_key_return_type])
        # Construct function name.
        buffer_name = ""
        buffer_name += dict_function[function_key_name] + '('
        arguments = dict_function[function_key_arguments]
        if not arguments:
            buffer_name += "void"
        else:
            buffer_name += ', '.join(arguments)
        buffer_name += ')'
        # Add function name.
        output_add(buffer_name)
        # Open function scope.
        output_add("{")
        # Add body.
        output_add('\n'.join(dict_function[function_key_body]))
        # Close function scope.
        output_add("}")
        output_sep()

    imports = [
        "stdlib",
        "stdio",
    ]

    output_add(format_imports(imports))
    output_sep()

    main = function_get("int", "main")
    function_add(main, printf("HELLO WORLD!"))
    function_add(main, printf("HELLO WORLD 2!"))
    function_add(main, printf("HELLO %s %d!", ["CUSTOM WORLD!", 4]))

    output_dump_function(main)

    output_print()

if __name__ == "__main__":
        main()
