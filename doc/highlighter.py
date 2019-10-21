TO_HIGHLIGHT = {"static", "void", "Vec2", "Vec3", "Vec4", "f32"}

def highlight_code(line):
    """ Adds orange color to words marked to be highlighted """
    def highlight(word):
        if word in TO_HIGHLIGHT:
            return '<span style="color: #f80">' + word + '</span>'
        return word

    return "".join([highlight(w) + s for w, s in zip(*split_all(line, " ,;()"))])


def split_all(string, sep):
    """
    Split a string at all characters in sep.
    Return two lists:
        words - Everything not including characters in sep
        splits - Everything including characters in sep

    Example:
        assert split_all("[1, 2]", " ,") == (["[1", "2]"], [", ", ""])

    The following is always true:
        len(words) == len(splits)
        len(words) > 0
        string == "".join([w + s for w, s in zip(*split_all(string, sep))])

    :param string str: String to split
    :param sep str: Characters where string should be split
    """
    words, splits = [], []
    word, split = "", ""
    i = 0
    while i < len(string):
        if string[i] in sep:
            words.append(word)
            word = ""
            while i < len(string) and string[i] in sep:
                split += string[i]
                i += 1
            splits.append(split)
            split = ""
            if i == len(string): break
        word += string[i]
        i += 1

    if word or not words:
        words.append(word)
        splits.append("")

    return words, splits
