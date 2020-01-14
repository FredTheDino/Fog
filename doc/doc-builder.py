#!/usr/bin/python3
from os import listdir as ls
import os.path as path
from functools import reduce
from enum import Enum
from highlighter import highlight_code

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


# Used to describe what is in this file, the first line is also
# mined for a name.
HEADING = 1  
DOC = 2  # API referend.
COMMENT = 3  # Misc comments.


def find_comments(file_path):
    """
    Parses out the comments of a file, categorizing them and making stripping
    them of unnessecary whitespace.
    """
    with open(file_path) as f:
        appending_to_comment = False
        heading = ""
        comments = []
        current_type = None
        comment = ""
        for line in f:
            if not appending_to_comment:
                next_type = None
                if "///#" in line:
                    next_type = HEADING
                    heading = line[4:].strip()
                elif "///*" in line:
                    next_type = DOC
                elif "////" in line:
                    next_type = COMMENT

                if next_type is not None:
                    appending_to_comment = True
                    if current_type is not None:
                        comments.append((current_type, comment))
                    current_type = next_type
                    comment = ""
                    if current_type != HEADING:
                        comment = line

            elif appending_to_comment:
                if line.strip() == "":
                    appending_to_comment = False
                else:
                    comment += line
    if comment:
        comments.append((current_type, comment))
    return heading, comments


def find_all_comments(files):
    """
    Finds all comments in a list of files and creates a documentation "tree"
    where the order of the modules are keept.
    Kinda structured like this: 
        [modules { submodules: [comments] }]
    Making it easy to create the documentation table and structure the
    comments.
    """
    # TODO(ed): This can probably be a list comphrehension.
    regions = []
    for region, _ in files:
        if region not in regions:
            regions.append(region)
    documentation = {region: dict() for region in regions}
    for region, file_path in sorted(files):
        heading, comments = find_comments(file_path)
        if heading and comments:
            documentation[region][heading] = comments 
    return [(region, documentation[region]) for region in reversed(regions)]


def tag(tag, content, html_class="", html_id=""):
    """
    Surrounds a piece of content in an html tag, with optional added
    class.
    """
    html_class = " class='{}'".format(html_class) if html_class else ""
    html_id = " id='{}'".format(html_id) if html_id else ""
    return "<{}{}{}>{}</{}>".format(tag, html_id, html_class, content, tag)


def link(text, to):
    """
    Creates a hyper link with the text to the position supplied.
    """
    return "<a href=\"{}\">{}</a>".format(to, text)


def make_id_friendly(string):
    """
    Returns the string but made into something that can be used
    as an ID.
    """
    from re import sub
    return sub(r"[^a-z0-9]", "", string.lower())


def process_comment_section(lines):
    """
    Parses out code from text and applies the appropriate markup.
    """
    out = ""
    in_comment = False
    for line in lines: 
        if line.strip() == "": continue
        if not in_comment and line.startswith("//"):
            in_comment = True
            if out:
                out += "</p>"
            out += "<p>"
        if in_comment and not line.startswith("//"):
            in_comment = False
            out += "</p>"
            out += "<p class='code'>"
        if in_comment:
            to_append = " " + line.replace("//", "").strip()
            if to_append:
                out += to_append
            else:
                out += "</p><p>"
        else:
            indent = len(line) - len(line.lstrip())
            out += "<span indent=\"{}\"></span>".format("#" * indent)
            safe_code = line.replace("<", "&lt;").replace(">", "&gt;").lstrip()
            out += highlight_code(safe_code)
            out += "<br>"
    return out.replace("<p></p>", "").strip()


def find_comment_title(comment):
    """
    Finds the title of a comment.
    """
    return comment[5:comment.index("\n")].capitalize()


def find_comment_id(section, comment):
    return make_id_friendly(section + find_comment_title(comment))


def format_comment(section, comment):
    """
    Formats the code according to how a comment should be formatted.
    """
    title = find_comment_title(comment)
    return tag("div", tag("h3", title) + process_comment_section(comment.split("\n")[1:]),
               "block comment",
               find_comment_id(section, comment))


def find_documentation_title(heading, comment):
    """
    Finds the title for this piece of documentation.
    """
    for line in comment.split("\n"): 
        if "///*" in line:
            potential_title = line[5:].strip()
            if potential_title:
                return heading.title() + ": " + potential_title
        for word in line.split(" "):
            if "(" in word:
                return heading.title() + ": " + word[:word.index("(")].replace("*", "")
    return "ERROR-NO-TITLE"


def find_documentation_id(section, comment):
    for line in comment.split("\n"): 
        if "///*" in line:
            potential_title = line[5:].strip()
            if potential_title:
                return potential_title
        if "(" in line and not "//" in line:
            return make_id_friendly(line)
    return "ERROR-NO-ID"


def format_documentation(section, comment):
    """
    Formants the code according to how a comment should be formatted.
    """
    title = find_documentation_title(section, comment)
    return tag("div", tag("h3", title) + process_comment_section(comment.split("\n")[1:]),
               "block doc",
               find_documentation_id(section, comment))


def format_heading(heading, comment):
    return tag("h2", heading, "section heading", make_id_friendly(heading)) + \
           tag("p", comment.replace("///#", "").replace("//", "").strip())


def has_content(region_headings):
    for heading in region_headings: 
        for comment_type, comment in region_headings[heading]:
            if comment:
                return True
    return False


def write_documentation(path, documentation):
    with open(path, "w") as f:
        PREAMBLE = "<html><head><title>Fog - Documentation</title><meta charset=utf-8><script src=\"script.js\"></script><link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"></head><body>"
        f.write(PREAMBLE)
        # Writing nav
        f.write("<nav><h2>Content</h2>")
        f.write("<ul id=\"nav\">")
        for region, headings in documentation:
            if not has_content(headings): continue
            f.write(tag("li", link(region.capitalize(), "#" + region), "hide hideable"))
            f.write("<li><ul>")
            for heading in headings:
                f.write(tag("li", link(heading, "#" + make_id_friendly(heading)),  "hide hideable"))
                f.write("<li><ul>")
                for comment_type, comment in headings[heading]:
                    if not comment: continue

                    if comment_type == HEADING:
                        continue
                    elif comment_type == DOC:
                        text = find_documentation_title(heading, comment)
                        html_id = find_documentation_id(heading, comment)
                    elif comment_type == COMMENT:
                        text = find_comment_title(comment)
                        html_id = find_comment_id(heading, comment)
                    f.write(tag("li", link(text, "#" + html_id)))
                f.write("</li></ul>")
                    
            f.write("</li></ul>")
        f.write("</ul>")
        f.write("</nav>")

        f.write("<article>")
        for region, headings in documentation:
            if not has_content(headings): continue
            f.write(tag("h1", region.capitalize(), "region heading", region))
            for heading in headings:
                for comment_type, comment in headings[heading]:
                    if not comment: continue

                    # Formats the comments to a more suitable HTML format.
                    if comment_type == HEADING:
                        output = format_heading(heading, comment)
                    elif comment_type == DOC:
                        output = format_documentation(heading, comment)
                    elif comment_type == COMMENT:
                        output = format_comment(heading, comment)
                    f.write(output)
        f.write("</article>")
        f.write("</body></html>")


if __name__ == "__main__":
    all_files = search("core", "src/")
    documentation = find_all_comments(all_files)
    write_documentation("doc/doc.html", documentation)
