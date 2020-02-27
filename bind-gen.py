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
    Find the definitons and put them in a nice list.
    """
    with open(file_path) as f:
        append_to_def = False
        defs = []
        definition = ""
        namespace = ""

        def add_defs(namespace, definition):
            nonlocal defs
            for func in definition.split(";"):
                if func.strip():
                    defs.append((func + ";", namespace))

        for line in f:
            if not append_to_def:
                add_defs(namespace, definition)
                definition = ""

                if "namespace" in line and not defs:
                    words = line.split()
                    if words[0] == "namespace" and len(words) <= 3:
                        namespace = words[1]
                elif "///*" in line:
                    append_to_def = True

            elif append_to_def:
                if line.strip() == "":
                    append_to_def = False
                elif line.startswith("//"):
                    continue
                else:
                    definition += line


        add_defs(namespace, definition)
    return defs

def find_typedefs(file_path):
    """
    Find the type defs
    """
    defs = []
    with open(file_path) as f:
        for line in f:
            if line.startswith("typedef"):
                defs.append(line.strip())
    return defs


def find_all_defs(files):
    """
    Finds all definitons in a list of files and creates a definition "tree"
    where the order of the modules are keept.
    Kinda structured like this:
        [modules { submodules: [definitons] }]
    Making it easy to create the definitons in a nice human readable
    format.
    """
    # TODO(ed): This can probably be a list comphrehension.
    regions = []
    for region, _ in files:
        if region not in regions:
            regions.append(region)
    documentation = {region: [] for region in regions}
    types = []
    for region, file_path in sorted(files):
        defs = find_defs(file_path)
        types += find_typedefs(file_path)
        if defs:
            documentation[region] += defs
    types = sorted(types, key=lambda s: "_t " not in s)
    return [(region, documentation[region]) for region in reversed(regions)], types

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

def format_function_def(definition, namespace):
    name = find_func_name(definition, namespace)
    if name is None:
        return None
    new_name = gen_new_name(name, namespace)
    if "*" in name:
        new_name = "*" + new_name
    return "FOG_EXPORT\n" + definition.replace(name, new_name).strip()

def get_args(definition, name):
    paren = definition.split(name)[1]
    assert paren[0] == "(", "Invalid arguments"
    inside_paren = ""
    ignore = False
    depth = 0
    for c in paren[1:]:
        if c == "=":
            ignore = True
        if c == "," and depth == 0:
            ignore = False
        if c == "(":
            depth += 1
        if c == ")":
            depth -= 1
        if not ignore:
            if c == ")":
                break
            inside_paren += c
    inside_paren = paren[paren.index("(")+1:paren.index(")")]
    return ", ".join([section.split("=")[0].strip() for section in inside_paren.split(",")])

def strip_types(args):
    if args:
        return ", ".join([arg.split()[-1].replace("*", "") for arg in args.split(",")])
    return ""

def write_function(definition, namespace):
    name = find_func_name(definition, namespace)
    if name is None:
        return None
    ptr = "*" * ("*" in name)
    new_name = gen_new_name(name, namespace)
    args = get_args(definition, name)
    args_no_type = strip_types(args)
    if namespace:
        func = "FOG_EXPORT\n" + definition.split(name)[0] + ptr + new_name + "(" + args + ") { return " + namespace + "::" + name[len(ptr):] + "(" + args_no_type + "); }"
    else:
        func = "FOG_EXPORT\n" + definition.split(name)[0] + ptr + new_name + "(" + args + ") { return " + name[len(ptr):] + "(" + args_no_type + "); }"
    return func

if __name__ == "__main__":
    all_files = search("core", "src/")
    all_defs, all_types = find_all_defs(all_files)

    bodies = []
    heads = []
    for region, defs in all_defs:
        for elem in defs:
            head = format_function_def(*elem)
            res = write_function(*elem)
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
    preamble += "#define FOG_EXPORT extern \"C\"\n"
    preamble += "#else __cplusplus\n"
    preamble += "#define FOG_EXPORT\n"
    preamble += "#endif\n\n"
    if heads:
        with open("bindings.cpp", "w") as f:
            f.write(preamble)
            f.write("\n".join(bodies))
            f.write("\n#undef FOG_EXPORT")
        with open("bindings.h", "w") as f:
            f.write(preamble)
            f.write("#include <stdint.h>\n")
            f.write("#ifndef __cplusplus\n")
            f.write("#define bool int\n")
            f.write("#define true 1\n")
            f.write("#define false 0\n")
            f.write("#endif\n")
            f.write("\n".join(all_types) + "\n\n")
            f.write("\n".join(heads))
            f.write("\n#undef FOG_EXPORT")
        print("Successfully generated bindings")
