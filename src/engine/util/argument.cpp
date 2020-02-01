namespace Util {

Argument parse_str_argument(char *input) {
    if (str_eq(input, "--resolution") || str_eq(input, "-r")) return resolution;
    return INVALID;
}

}  // namespace Util
