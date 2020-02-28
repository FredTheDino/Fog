#!/usr/bin/python3
from os import listdir as ls
import os.path as path
from functools import reduce
from enum import Enum

def search(region, root):
    """
    Finds all the subdirectories of a directory, and pairs them with the name
    of their parent directory.
    """
    files_in_lists = [search(f, path.join(root, f)) for f in ls(root)
                                    if path.isdir(path.join(root, f))] +\
                     [[(region, path.join(root, f)) for f in ls(root)
                                    if path.isfile(path.join(root, f))]]
    return reduce(lambda a, b: a + b, files_in_lists)


def find_defs(file_path):
    """
    Find the definitons and put them in a nice list,
    if none are found, an empty list is returned.
    """
    def export_struct(lines, namespace):
        struct = ""
        rest = lines
        depth = 0
        while rest:
            line = rest[0]
            rest = rest[1:]
            struct += line
            if "{" in line:
                depth += 1
            if "}" in line:
                depth -= 1
                if depth == 0:
                    break

        assert depth == 0, "Invalid bracing."
        return [(namespace, "STRUCT", struct)] + parse(rest, namespace)

    def export_typedef(lines, namespace):
        line = lines[0]
        rest = lines[1:]
        return [(namespace, "EXPORT", line)] + parse(rest, namespace)

    def export_func(lines, namespace):
        func = ""
        rest = lines
        while rest:
            line = rest[0]
            rest = rest[1:]
            if line.strip() == "":
                break
            if "//" in line or "#" in line:
                continue
            if not " " in line:
                continue
            func += line


        result = [(namespace, "FUNC", f.strip() + ";") for f in func.split(";")]
        return result + parse(rest, namespace)

    def hide(lines, namespace):
        rest = lines
        while rest:
            line = rest[0]
            rest = rest[1:]
            if line.strip() == "":
                break
        return parse(rest, namespace)


    def parse(lines, namespace):
        if not lines:
            return []
        line = lines[0]
        rest = lines[1:]
        if "namespace" in line:
            words = line.split()
            if words[0] == "namespace" and len(words) <= 3:
                namespace = words[1]
        if "#" in line:
            return parse(rest, namespace)
        if "///*" in line:
            return export_func(rest, namespace)
        if "FOG_EXPORT_STRUCT" in line:
            return export_struct(rest, namespace)
        if "FOG_EXPORT" in line:
            return export_typedef(rest, namespace)
        if "FOG_HIDE" in line:
            return hide(rest, namespace)
        return parse(rest, namespace)

    with open(file_path) as f:
        return parse(f.readlines(), "")
    print("Failed to open file:", file_path)
    return []

def find_all_defs(files):
    """
    Finds all definitons in a list of files and creates a definition "tree"
    where the order of the modules are keept.
    Kinda structured like this:
        [modules { submodules: [definitons] }]
    Making it easy to create the definitons in a nice human readable
    format.
    """
    documentation = []
    for _, file_path in sorted(files):
        documentation += find_defs(file_path)
    return filter(lambda a: len(a[2]) > 3, documentation)

GLOBAL_NAMESPACE = "fog_"
import sys
def err(*args, **kvargs):
    print(*args, file=sys.stderr, **kvargs)


def find_func_name(definition, namespace):
    possible_names = list(filter(lambda s: "(" in s and s[0] != "(", definition.split(" ")[1:]))
    if not possible_names:
        return None
    return possible_names[0].split("(")[0]

def gen_new_name(name, namespace):
    if namespace:
        return GLOBAL_NAMESPACE + namespace.lower() + "_" + name.replace("*", "")
    return GLOBAL_NAMESPACE + name.replace("*", "")

def format_function_def(namespace, definition):
    name = find_func_name(definition, namespace)
    if name is None:
        return None
    new_name = gen_new_name(name, namespace)
    if "*" in name:
        new_name = "*" + new_name
    return "FOG_IMPORT\n" + definition.replace(name, new_name).strip()

def get_args(original):
    args = ""
    ignoring = False
    depth = 0
    for c in original:
        if c == "=":
            ignoring = True
        if c == "(":
            depth += 1
            continue
        if c == ")":
            depth -= 1
            if not depth:
                break
        if depth == 0 and c == ",":
            ignoring = False
        if not ignoring:
            args += c
    if args:
        return ", ".join([arg.strip() for arg in args.split(",")])
    return ""

def strip_type(args):
    if args:
        return ", ".join([arg.split()[-1].replace("*", "") for arg in args.split(",")])
    return ""

def write_function(namespace, definition):
    name = find_func_name(definition, namespace)
    if name is None:
        return None
    ptr = "*" * ("*" in name)
    new_name = gen_new_name(name, namespace)
    args_raw = definition.split(name)[1]
    args = get_args(args_raw)
    args_no_type = strip_type(args)
    func = "FOG_IMPORT\n"
    func += definition.split(name)[0]
    func += ptr
    func += new_name
    func += "("
    func += args
    func += ") { "
    if "..." in args:
        func += "va_list _l; va_start(_l, " + args_no_type.split(",")[-2] + "); "
        func += "auto _r = "
    else:
        func += "return "

    if namespace:
        func += namespace
        func += "::"
    func += name[len(ptr):]
    func += "("
    func += args_no_type.replace("...", "_l")
    func += "); "
    if "..." in args:
        func += "return _r; "
    func += "}"
    return func

if __name__ == "__main__":
    all_files = search("core", "src/")
    all_defs = find_all_defs(all_files)
    def sorting_key(elem):
        _, kind, defs = elem
        if kind == "EXPORT":
            if "_t" in defs:
                return 0
            if "Vec" in defs:
                return 2
            if "const " in defs:
                return 5
            return 4
        if kind == "STRUCT":
            if "struct Vec" in defs:
                return 1
            return 3
        return 10

    all_defs = sorted(all_defs, key=sorting_key)

    bodies = []
    heads = []
    namespaces = set()
    for elem in all_defs:
        namespace, kind, source = elem
        if namespace:
            namespaces.add(namespace)
        if kind == "STRUCT" or kind == "EXPORT":
            heads.append(source)
        else:
            head = format_function_def(elem[0], elem[2])
            res = write_function(elem[0], elem[2])
            if res is not None and head is not None:
                heads.append(" ".join(head.split()))
                bodies.append(" ".join(res.split()))
            else:
                err("Failed to parse", elem, "got: \n", head, "\n", res)
    preamble = "// Hi!\n"
    preamble += "// This file is automatically generated C-bindings for the Fog engine,\n"
    preamble += "// if you are planning on editing this, it will probably be overwritten\n"
    preamble += "// on the next compilation. So maybe don't put stuff in here?\n"
    preamble += "\n"
    preamble += "#ifdef __cplusplus\n"
    preamble += "#define FOG_IMPORT extern \"C\"\n"
    preamble += "#else\n"
    preamble += "#define FOG_IMPORT\n"
    preamble += "#endif\n\n"
    if heads:
        with open("src/fog_bindings.cpp", "w") as f:
            f.write(preamble)
            f.write("\n".join(["using namespace " + ns + ";" for ns in namespaces]))
            f.write("\n")
            f.write("\n".join(bodies))
            f.write("\n#undef FOG_IMPORT")
        with open("out/fog.h", "w") as f:
            f.write(preamble)
            f.write("#include <stdint.h>\n")
            f.write("#ifndef __cplusplus\n")
            f.write("#define bool int\n")
            f.write("#define true 1\n")
            f.write("#define false 0\n")
            f.write("#endif\n")
            f.write("\n".join(heads))
            f.write("\n#undef FOG_IMPORT")
    else:
        sys.exit(-1)
