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
    for region, file_path in sorted(files):
        defs = find_defs(file_path)
        if defs:
            documentation[region] = defs
    return [(region, documentation[region]) for region in reversed(regions)]

GLOBAL_NAMESPACE = "fog_"
import sys
def err(*args, **kvargs):
    print(*args, file=sys.stderr, **kvargs)

if __name__ == "__main__":
    all_files = search("core", "src/")
    all_defs = find_all_defs(all_files)
    for region, defs in all_defs:
        print("//", region)

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
                # TODO(ed): Log out the name that failed.
                err("Failed to parse (got: ", name, ") from\n", definition)
                return None
            new_name = gen_new_name(name, namespace)
            if "*" in name:
                new_name = "*" + new_name
            return definition.replace(name, new_name).strip()

        def get_args(definition, name):
            paren = definition.split(name)[1]
            inside_paren = paren[paren.index("(")+1:paren.index(")")]
            return ", ".join([section.split("=")[0].strip() for section in inside_paren.split(",")])

        def write_function(definition, namespace):
            name = find_func_name(definition, namespace)
            if name is None:
                # TODO(ed): Log out the name that failed.
                print
                return None
            ptr = "*" * ("*" in name)
            new_name = gen_new_name(name, namespace)
            args = get_args(definition, name)
            if namespace:
                func = definition.split(name)[0] + ptr + new_name + "(" + args + ") { return " + namespace + "::" + name[len(ptr):] + "(" + args + "); }"
            else:
                func = definition.split(name)[0] + ptr + new_name + "(" + args + ") { return " + name[len(ptr):] + "(" + args + "); }"
            return func

        result = []
        for elem in defs:
            res = write_function(*elem)
            if res is not None:
                result.append(" ".join(res.split()))
        if result:
            print("\n".join(result))
