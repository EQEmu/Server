import os
import re

def extract_variable_name(var_expr):
    # Remove strings and character literals to avoid interference
    var_expr_no_strings = re.sub(r'"[^"]*"|\'[^\']*\'', '', var_expr)

    # Replace function calls with their names
    var_expr_no_strings = re.sub(r'\b(\w+)\s*\([^()]*\)', r'\1', var_expr_no_strings)

    # Extract variable-like tokens, including member access and pointers
    tokens = re.findall(r'\b(?:\w+)(?:->\w+|\.\w+)+\b', var_expr_no_strings)

    # If no tokens with member access, include standalone variables
    if not tokens:
        tokens = re.findall(r'\b\w+\b', var_expr_no_strings)

    # Prioritize tokens with '->' or '.'
    if tokens:
        # Choose the longest token
        variable_name = max(tokens, key=len)
    else:
        variable_name = slugify(var_expr)

    return variable_name

def slugify(var_expr):
    # Remove any quotes
    var_expr = var_expr.replace('"', '').replace("'", '')
    # Replace non-alphanumeric characters with underscores
    var_expr = re.sub(r'\W+', '_', var_expr)
    # Remove leading/trailing underscores
    var_expr = var_expr.strip('_')
    # Optionally limit the length
    var_expr = var_expr[:50]  # limit to 50 characters
    return var_expr

def process_file(file_path):
    with open(file_path, 'r') as f:
        content = f.read()

    original_content = content

    # Regular expression to match Log macros
    log_macro_pattern = re.compile(r'(Log\w*\s*\()([^\n]*?\))', re.DOTALL)

    def replace_log(match):
        log_call = match.group(1)
        args_content = match.group(2)

        # Split arguments, handling nested structures
        args = split_arguments(args_content.strip(')'))

        if len(args) < 2:
            # Not enough arguments to process
            return match.group(0)

        format_string = args[0].strip()
        variables = args[1:]

        # Remove quotes from format string
        if (format_string.startswith('"') and format_string.endswith('"')) or \
                (format_string.startswith("'") and format_string.endswith("'")):
            quote_char = format_string[0]
            format_string_content = format_string[1:-1]
        else:
            # Format string is not properly quoted
            return match.group(0)

        # Find all '{}' placeholders in format string
        placeholder_pattern = re.compile(r'(?<!{){(?!{)(?![^}]*:)[^}]*}(?!})')
        placeholders = list(placeholder_pattern.finditer(format_string_content))

        # Replace '{}' placeholders with '[variable|{}]' or 'variable|{}' depending on context
        new_format_string_content = ''
        last_index = 0
        arg_index = 0

        for ph in placeholders:
            start, end = ph.span()
            preceding_text = format_string_content[max(0, start-1):start]
            following_text = format_string_content[end:end+1]
            new_format_string_content += format_string_content[last_index:start]

            # Only process '{}' placeholders
            if format_string_content[start:end] == '{}':
                if arg_index < len(variables):
                    variable = variables[arg_index].strip()
                    arg_index += 1

                    # Extract meaningful variable name
                    variable_name = extract_variable_name(variable)

                    # Check if '{}' is already inside brackets
                    inside_brackets = (
                            (preceding_text == '[' and following_text == ']') or
                            (preceding_text == '[') or
                            (following_text == ']')
                    )

                    if inside_brackets:
                        # Replace '{}' with 'variable_name|{}' without adding extra brackets
                        new_placeholder = f'{variable_name}|{{}}'
                    else:
                        # Add brackets around 'variable_name|{}'
                        new_placeholder = f'[{variable_name}|{{}}]'
                    new_format_string_content += new_placeholder
                else:
                    # No corresponding variable; leave '{}' as is
                    new_format_string_content += '{}'
            else:
                # Keep the original placeholder (e.g., {:#04x})
                new_format_string_content += format_string_content[start:end]

            last_index = end

        new_format_string_content += format_string_content[last_index:]

        # Reconstruct the format string with original quotes
        new_format_string = quote_char + new_format_string_content + quote_char

        # Reconstruct the arguments
        new_args_content = ', '.join(args[1:])  # Exclude the format string

        # Reconstruct the log call
        new_log_call = f'{log_call}{new_format_string}'
        if new_args_content.strip():
            new_log_call += f', {new_args_content}'
        new_log_call += ')'

        return new_log_call

    def split_arguments(args_str):
        # This function splits arguments, handling nested structures
        args = []
        current_arg = ''
        depth = 0
        in_string = False
        escape = False
        i = 0
        length = len(args_str)
        while i < length:
            char = args_str[i]
            if escape:
                current_arg += char
                escape = False
            elif char == '\\':
                current_arg += char
                escape = True
            elif char in ('"', "'"):
                current_arg += char
                if in_string == char:
                    in_string = False
                elif not in_string:
                    in_string = char
            elif not in_string:
                if char in '([{':
                    depth += 1
                    current_arg += char
                elif char in ')]}':
                    depth -= 1
                    current_arg += char
                elif char == ',' and depth == 0:
                    args.append(current_arg.strip())
                    current_arg = ''
                else:
                    current_arg += char
            else:
                current_arg += char
            i += 1
        if current_arg.strip():
            args.append(current_arg.strip())
        return args

    # Replace all Log macros in the content
    content = log_macro_pattern.sub(replace_log, content)

    if content != original_content:
        # Write the modified content back to the file without creating backups
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"Processed {file_path}")

def main():
    for root, dirs, files in os.walk('.'):
        for filename in files:
            if filename.endswith('.cpp'):
                file_path = os.path.join(root, filename)
                process_file(file_path)

if __name__ == '__main__':
    main()
