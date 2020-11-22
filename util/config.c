/*
 * Copyright (c) 2020 Martin Rödel a.k.a. Yomin Nimoy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/// \file

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>


/// config option type
typedef enum config_type
{
    CONFIG_TYPE_UNSET,      ///< unset option
    CONFIG_TYPE_TOGGLE,     ///< toggle option
    CONFIG_TYPE_BOOL,       ///< boolean option
    CONFIG_TYPE_INT,        ///< signed integer option
    CONFIG_TYPE_UINT,       ///< unsigned integer option
    CONFIG_TYPE_FLOAT,      ///< float option
    CONFIG_TYPE_TEXT,       ///< text option
    CONFIG_TYPES,           ///< number of option types
} config_type_id;

/// config option type names
static const char *config_types[] =
{
    [CONFIG_TYPE_TOGGLE]    = "toggle",
    [CONFIG_TYPE_BOOL]      = "bool",
    [CONFIG_TYPE_INT]       = "int",
    [CONFIG_TYPE_UINT]      = "uint",
    [CONFIG_TYPE_FLOAT]     = "float",
    [CONFIG_TYPE_TEXT]      = "text",
};

/// config error
typedef enum config_error
{
    E_CONFIG_SUCCESS,           ///< success
    E_CONFIG_INVALID_TYPE,      ///< invalid type
    E_CONFIG_MISSING_TYPE,      ///< missing type
    E_CONFIG_INVALID_VALUE,     ///< invalid value
    E_CONFIG_INVALID_DEP,       ///< invalid dependency
    E_CONFIG_UNKNOWN_DEP,       ///< unknown dependency
} config_error_id;

/// config operation
typedef enum config_op
{
    CONFIG_OP_BEGIN,    ///< begin operation
    CONFIG_OP_OPTION,   ///< option operation
    CONFIG_OP_END,      ///< end operation
} config_op_id;

/// config option
typedef struct config_option
{
    char                    *name;      ///< option name
    char                    *desc;      ///< option description
    config_type_id          type;       ///< option type
    char                    *def;       ///< option default value
    char                    *value;     ///< option value
    struct config_option    **deps;     ///< option dependencies, NULL terminated
} config_option_st;

/// config options
typedef struct config_options
{
    config_option_st    *list;  ///< option list
    size_t              size;   ///< number of config options
    size_t              cap;    ///< capacity of option list
} config_options_st;

static config_options_st options;   ///< config options state
static bool verbose;                ///< verbose printing
static bool interactive;            ///< interactive configuration

/// config import callback
///
/// \param file     imported file
/// \param line     line in file
/// \param op       operation
/// \param key      option key, NULL if op != CONFIG_OP_OPTION
/// \param value    option value, NULL if op != CONFIG_OP_OPTION
///
/// \retval 0   success
/// \retval -1  error
typedef int (*config_import_cb)(const char *file, size_t line, config_op_id op, char *key, char *value);

/// config export callback
///
/// \param file     exported file
/// \param fp       file stream
/// \param op       operation
/// \param option   option to export, NULL if op != CONFIG_OP_OPTION
///
/// \retval 0   success
/// \retval -1  error
typedef int (*config_export_cb)(const char *file, FILE *fp, config_op_id op, const config_option_st *option);


/// Free all options.
///
///
static void config_options_free(void)
{
    size_t opt;

    for(opt = 0; opt < options.size; opt++)
    {
        free(options.list[opt].name);
        free(options.list[opt].desc);
        free(options.list[opt].def);
        free(options.list[opt].value);
        free(options.list[opt].deps);
    }

    free(options.list);
    memset(&options, 0, sizeof(config_options_st));
}

/// Add new option.
///
/// \param name     option name
///
/// \returns        option
/// \retval NULL    error
static config_option_st *config_option_add(const char *name)
{
    config_option_st *option;

    if(!options.list || options.size >= options.cap)
    {
        if(!(option = realloc(options.list, (options.cap + 10) * sizeof(config_option_st))))
            return perror("failed to alloc options"), NULL;

        options.list    = option;
        options.cap     += 10;
    }

    option = &options.list[options.size];
    memset(option, 0, sizeof(config_option_st));

    if(!(option->name = strdup(name)))
        return perror("failed to alloc option"), NULL;

    options.size++;

    return option;
}

/// Set option description.
///
/// \param option   option
/// \param desc     description
///
/// \retval 0       success
/// \retval -1      error
static int config_option_set_desc(config_option_st *option, const char *desc)
{
    size_t size;
    char *ddesc;

    size = (option->desc ? strlen(option->desc) + 1 : 0) + strlen(desc);

    if(!(ddesc = calloc(1, size + 1)))
        return perror("failed to alloc option"), -1;

    if(option->desc)
        sprintf(ddesc, "%s\n%s", option->desc, desc);
    else
        strcpy(ddesc, desc);

    free(option->desc);
    option->desc = ddesc;

    return 0;
}

/// Count character occurences in string.
///
/// \param str      string
/// \param c        character
///
/// \returns        number of \p c occurences in \p str
static size_t strcnt(const char *str, int c)
{
    size_t count;

    for(count = 0; str[0]; str++)
    {
        if(str[0] == c)
            count++;
    }

    return count;
}

/// Set option type.
///
/// \param option   option
/// \param type     type string
///
/// \retval 0                       success
/// \retval E_CONFIG_INVALID_TYPE   invalid type
static int config_option_set_type(config_option_st *option, const char *type)
{
    config_type_id ttype;

    for(ttype = CONFIG_TYPE_UNSET + 1; ttype < CONFIG_TYPES; ttype++)
    {
        if(!strcmp(type, config_types[ttype]))
            break;
    }

    if(ttype == CONFIG_TYPES)
        return E_CONFIG_INVALID_TYPE;

    option->type = ttype;

    return 0;
}

/// Check value against type.
///
/// \param type     type ID
/// \param value    value string
///
/// \retval true    \p text is valid
/// \retval false   \p text is not valid
static bool config_value_is_valid(config_type_id type, const char *value)
{
    switch(type)
    {
    case CONFIG_TYPE_TOGGLE:
        return !strcmp(value, "on") || !strcmp(value, "off");

    case CONFIG_TYPE_BOOL:
        return !strcmp(value, "true") || !strcmp(value, "false");

    case CONFIG_TYPE_INT:

        if(value[0] == '-')
            value++;

        __attribute__((fallthrough));

    case CONFIG_TYPE_UINT:
        return value[0] && strspn(value, "1234567890") == strlen(value);

    case CONFIG_TYPE_FLOAT:
        return value[0] && strcnt(value, '.') <= 1
               && strspn(value, "1234567890.") == strlen(value);

    case CONFIG_TYPE_TEXT:

        for(; value[0]; value++)
        {
            if(!isprint(value[0]) || value[0] == '"')
                return false;

            if(value[0] != '\\')
                continue;

            value++;

            if(!value[0])
                return false;

            if(value[0] != 'x' && value[0] != 'X')
                continue;

            if(!isxdigit(value[1]) || !isxdigit(value[2]))
                return false;

            value += 2;
        }

        return true;

    default:
        abort();
    }
}

/// Set option value.
///
/// \param option   option
/// \param value    value string
/// \param def      if true value is default value
///
/// \retval 0                       success
/// \retval -1                      error
/// \retval E_CONFIG_INVALID_VALUE  invalid value for option type
static int config_option_set_value(config_option_st *option, const char *value, bool def)
{
    size_t size;
    char **ovalue, *dvalue;

    if(!option->type)
        return E_CONFIG_MISSING_TYPE;

    if(!config_value_is_valid(option->type, value))
        return E_CONFIG_INVALID_VALUE;

    ovalue = def ? &option->def : &option->value;

    if(option->type == CONFIG_TYPE_TEXT)
    {
        size = (*ovalue ? strlen(*ovalue) + 1 : 0) + strlen(value);

        if(!(dvalue = calloc(1, size + 1)))
            return perror("failed to alloc option"), -1;

        if(*ovalue)
            sprintf(dvalue, "%s\n%s", *ovalue, value);
        else
            strcpy(dvalue, value);
    }
    else
    {
        if(!(dvalue = strdup(value)))
            return perror("failed to alloc option"), -1;
    }

    free(*ovalue);
    *ovalue = dvalue;

    return 0;
}

/// Set option dependencies.
///
/// \param option   option
/// \param deps     dependencies string
///
/// \retval 0                       success
/// \retval -1                      error
/// \retval E_CONFIG_INVALID_DEP    invalid dependency
/// \retval E_CONFIG_UNKNOWN_DEP    unknown dependency
static int config_option_set_deps(config_option_st *option, char *deps)
{
    config_option_st **odeps, **odep, *opt;
    size_t excount = 0, count, o;
    char *dep;

    if(!deps[0])
    {
        free(option->deps);
        option->deps = NULL;
        return 0;
    }

    if(option->deps)
    {
        for(odep = option->deps; *odep; odep++)
            excount++;
    }

    count = strcnt(deps, ' ') + 2;

    if(!(odeps = realloc(option->deps, (excount + count) * sizeof(config_option_st *))))
        return perror("failed to alloc option"), -1;

    option->deps    = odeps;
    odep            = &odeps[excount];

    memset(odep, 0, count * sizeof(config_option_st *));

    for(dep = strtok(deps, " "); dep; dep = strtok(NULL, " "), odep++)
    {
        for(o = 0, opt = options.list; o < options.size; o++, opt++)
        {
            if(strcmp(opt->name, dep))
                continue;

            if(opt == option || opt->type != CONFIG_TYPE_TOGGLE)
                return memmove(deps, dep, strlen(dep) + 1), free(odeps), E_CONFIG_INVALID_DEP;

            odep[0] = opt;
            break;
        }

        if(o == options.size)
            return memmove(deps, dep, strlen(dep) + 1), free(odeps), E_CONFIG_UNKNOWN_DEP;
    }

    return 0;
}

/// Check if option is active (all dependencies are toggled on and active).
///
/// \param option   option
///
/// \retval true    option is active
/// \retval false   option is inactive
static bool config_option_is_active(const config_option_st *option)
{
    config_option_st **dep;
    char *value;

    if(!option->deps)
        return true;

    for(dep = option->deps; dep[0]; dep++)
    {
        value = dep[0]->value ? dep[0]->value : dep[0]->def;

        if(strcmp(value, "on") || !config_option_is_active(dep[0]))
            return false;
    }

    return true;
}

/// Print text property.
///
/// \param name     property name
/// \param text     text to print
/// \param indent   number of spaces to indent
/// \param width    name width
static void print_text_property(const char *name, const char *text, int indent, int width)
{
    int post_indent = width - strlen(name) - 1;
    const char *next;

    printf("%*s%s:%*s", indent, "", name, post_indent < 0 ? 0 : post_indent, "");

    for(; (next = strchr(text, '\n')); text = next + 1)
        printf("%.*s\n%*s", (int)(next - text), text, indent + width, "");

    printf("%s\n", text);
}

/// Print option properties to stdout.
///
/// \param option   option
/// \param indent   number of spaces to indent
static void config_option_print(const config_option_st *option, int indent)
{
    config_option_st **dep;

    printf("%*soption:   %s\n", indent, "", option->name);

    if(option->desc)
        print_text_property("desc", option->desc, indent, 10);

    printf("%*stype:     %s\n", indent, "", config_types[option->type]);
    print_text_property("default", option->def, indent, 10);

    if(option->value)
        print_text_property("value", option->value, indent, 10);

    if(option->deps)
    {
        printf("%*sdeps:     ", indent, "");

        for(dep = option->deps; dep[0]; dep++)
            printf("%s ", dep[0]->name);

        printf("\n");
    }
}

/// Import key value pair from template config file.
///
/// \implements config_import_cb
static int config_import_template(const char *file, size_t line, config_op_id op, char *key, char *value)
{
    static config_option_st *option = NULL;

    if(op == CONFIG_OP_BEGIN)
        return 0;

    if(op == CONFIG_OP_END || !strcmp(key, "option"))
    {
        if(option)
        {
            if(!option->type)
            {
                fprintf(stderr, "%s:%zu: option [%s] missing type\n",
                    file, line, option->name);
                return -1;
            }

            if(!option->def)
            {
                fprintf(stderr, "%s:%zu: option [%s] missing default value\n",
                    file, line, option->name);
                return -1;
            }

            if(verbose)
                printf("  Option [%s|%s] = %s\n", option->name, config_types[option->type], option->def);
        }

        if(op == CONFIG_OP_OPTION && !(option = config_option_add(value)))
            return -1;
    }
    else if(!option)
    {
        fprintf(stderr, "%s:%zu: missing option for key '%s'\n", file, line, key);
        return -1;
    }
    else if(!strcmp(key, "desc"))
    {
        if(config_option_set_desc(option, value))
            return -1;
    }
    else if(!strcmp(key, "type"))
    {
        if(config_option_set_type(option, value))
            return fprintf(stderr, "%s:%zu: invalid type '%s'\n", file, line, value), -1;
    }
    else if(!strcmp(key, "default") || !strcmp(key, "value"))
    {
        switch(config_option_set_value(option, value, true))
        {
        case 0:
            break;

        case E_CONFIG_MISSING_TYPE:
            fprintf(stderr, "%s:%zu: option [%s] need type to set value\n",
                file, line, option->name);
            return -1;

        case E_CONFIG_INVALID_VALUE:
            fprintf(stderr, "%s:%zu: invalid default value [%s] '%s'\n",
                file, line, config_types[option->type], value);
            return -1;

        default:
            return -1;
        }
    }
    else if(!strcmp(key, "deps"))
    {
        switch(config_option_set_deps(option, value))
        {
        case 0:
            break;

        case E_CONFIG_INVALID_DEP:
            fprintf(stderr, "%s:%zu: invalid dependency [%s]\n", file, line, value);
            return -1;

        case E_CONFIG_UNKNOWN_DEP:
            fprintf(stderr, "%s:%zu: unknown dependency [%s]\n", file, line, value);
            return -1;

        default:
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "%s:%zu: invalid key '%s'\n", file, line, key);
        return -1;
    }

    return 0;
}

/// Import key value pair from config file.
///
/// \implements config_import_cb
static int config_import_config(const char *file, size_t line, config_op_id op, char *key, char *value)
{
    config_option_st *option;
    size_t opt;

    if(op != CONFIG_OP_OPTION)
        return 0;

    for(opt = 0; opt < options.size; opt++)
    {
        option = &options.list[opt];

        if(strcmp(option->name, key))
            continue;

        switch(config_option_set_value(option, value, false))
        {
        case 0:

            if(verbose)
                printf("  Set [%s|%s] = %s\n", option->name, config_types[option->type], value);

            break;

        case E_CONFIG_INVALID_VALUE:
            fprintf(stderr, "%s:%zu: invalid value [%s] '%s'\n",
                file, line, config_types[option->type], value);

            if(!option->value)
            {
                fprintf(stderr, "%s:%zu: reset [%s] to default value (%s)\n",
                    file, line, key, option->def);
            }

            break;

        default:
            return -1;
        }

        return 0;
    }

    fprintf(stderr, "%s:%zu: ignore unknown option [%s]\n", file, line, key);

    return 0;
}

/// Remove blanks from begin and end of string.
///
/// \param str      string to trim
///
/// \returns        trimmed string
static char *trim(char *str)
{
    char *end = str ? str + strlen(str) : NULL;

    if(!str)
        return NULL;

    while(isspace(str[0]))
        str++;

    while(end > str && isspace(end[-1]))
        end--;

    end[0] = '\0';

    return str;
}

/// Import config file.
///
/// \param file     config file
/// \param optional if true ignore missing file
/// \param import   callback to invoke on each config key value pair
///
/// \retval 0       success
/// \retval -1      error
static int config_import(const char *file, bool optional, config_import_cb import)
{
    FILE *fp;
    char *line = NULL, *key, *value;
    size_t size = 0, lineno;

    if(!(fp = fopen(file, "rb")))
    {
        if(optional)
            return 0;
        else
            return fprintf(stderr, "%s: failed to open: %s\n", file, strerror(errno)), -1;
    }

    if(import(file, 0, CONFIG_OP_BEGIN, NULL, NULL))
        return fclose(fp), -1;

    for(lineno = 1, errno = 0; getline(&line, &size, fp) > 0; lineno++, errno = 0)
    {
        key     = line;
        value   = strchr(line, '=');

        if(value)
        {
            value[0] = '\0';
            value++;
        }

        key     = trim(key);
        value   = trim(value);

        if(key[0] && !value)
        {
            fprintf(stderr, "%s:%zu: missing value for '%s'\n", file, lineno, key);
            return free(line), fclose(fp), -1;
        }

        if(!key[0])
        {
            if(value)
            {
                fprintf(stderr, "%s:%zu: missing key for value '%s'\n", file, lineno, value);
                return free(line), fclose(fp), -1;
            }

            continue;
        }

        if(import(file, lineno, CONFIG_OP_OPTION, key, value))
            return free(line), fclose(fp), -1;
    }

    if(errno)
        return perror("failed to read line"), free(line), fclose(fp), -1;

    if(import(file, lineno, CONFIG_OP_END, NULL, NULL))
        return free(line), fclose(fp), -1;

    free(line);
    fclose(fp);

    return 0;
}

/// Export option to config file.
///
/// \implements config_export_cb
static int config_export_config(const char *file, FILE *fp, config_op_id op, const config_option_st *option)
{
    char *text, *next;
    int size;

    if(op != CONFIG_OP_OPTION || !option->value)
        return 0;

    for(text = option->value; (next = strchr(text, '\n')); text = next + 1)
    {
        size = next - text;
        fprintf(fp, "%s =%s%.*s\n", option->name, size ? " " : "", size, text);
    }

    fprintf(fp, "%s =%s%s\n", option->name, text[0] ? " " : "", text);

    return 0;
}

/// Generate include guard from file name.
///
/// \param file     file
///
/// \returns        alloced include guard
/// \retval -1      error
static char *guard_from_file(const char *file)
{
    char *guard, *ptr;

    if(!(guard = calloc(1, strlen(file) + strlen("_INCLUDED") + 1)))
        return perror("failed to alloc guard"), NULL;

    for(ptr = guard; file[0]; ptr++, file++)
        ptr[0] = isalnum(file[0]) ? toupper(file[0]) : '_';

    strcpy(ptr, "_INCLUDED");

    return guard;
}

/// Write formatted text to stream.
///
/// \param text     text
/// \param stream   stream
static void fputs_text(const char *text, FILE *stream)
{
    const char *next;

    if(strchr(text, '\n'))
        fputs("\\\n    ", stream);

    putc('"', stream);

    for(; (next = strchr(text, '\n')); text = next + 1)
        fprintf(stream, "%.*s\" \\\n    \"", (int)(next - text), text);

    fprintf(stream, "%s\"", text);
}

/// Export option to header file.
///
/// \implements config_export_cb
static int config_export_header(const char *file, FILE *fp, config_op_id op, const config_option_st *option)
{
    char *guard, *value, *text, *next;
    int size;

    switch(op)
    {
    case CONFIG_OP_BEGIN:

        if(!(guard = guard_from_file(file)))
            return -1;

        fprintf(fp, "\n// Automatically generated. Do not edit.\n\n");
        fprintf(fp, "#ifndef %s\n", guard);
        fprintf(fp, "#define %s\n\n", guard);

        free(guard);
        break;

    case CONFIG_OP_OPTION:

        if(option->desc)
        {
            for(text = option->desc; (next = strchr(text, '\n')); text = next + 1)
            {
                size = next - text;
                fprintf(fp, "//%s%.*s\n", size ? " " : "", size, text);
            }

            fprintf(fp, "//%s%s\n", text[0] ? " " : "", text);
        }

        value = option->value ? option->value : option->def;

        switch(option->type)
        {
        case CONFIG_TYPE_TOGGLE:

            if(strcmp(value, "on") || !config_option_is_active(option))
                fprintf(fp, "// #define %s", option->name);
            else
                fprintf(fp, "#define %s", option->name);

            break;

        case CONFIG_TYPE_TEXT:
            fprintf(fp, "%s#define %s ",
                config_option_is_active(option) ? "" : "// ", option->name);
            fputs_text(value, fp);
            break;

        default:
            fprintf(fp, "%s#define %s %s",
                config_option_is_active(option) ? "" : "// ", option->name, value);
        }

        fputs("\n\n", fp);
        break;

    case CONFIG_OP_END:
        fputs("#endif\n", fp);
        break;
    }

    return 0;
}

/// Create all directories up to file.
///
/// \param file     file
///
/// \retval 0       success
/// \retval -1      error
static int mkdir_for_file(const char *file)
{
    char *dir, *sep;

    if(!(dir = strdup(file)))
        return perror("failed to mkdir"), -1;

    for(sep = dir; (sep = strchr(sep, '/')); sep = sep + 1)
    {
        sep[0] = '\0';

        if(mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) && errno != EEXIST)
            return perror("failed to mkdir"), free(dir), -1;

        sep[0] = '/';
    }

    free(dir);

    return 0;
}

/// Export config file.
///
/// \param file     config file
/// \param export   callback to invoke on each config option
///
/// \retval 0       success
/// \retval -1      error
static int config_export(const char *file, config_export_cb export)
{
    FILE *fp;
    size_t opt;
    char *tmp;

    if(mkdir_for_file(file))
        return -1;

    if(!(tmp = calloc(1, strlen(file) + strlen(".tmp") + 1)))
        return perror("failed to export"), -1;

    strcpy(tmp, file);
    strcat(tmp, ".tmp");

    if(!(fp = fopen(tmp, "w")))
        return fprintf(stderr, "%s: failed to open: %s\n", tmp, strerror(errno)), free(tmp), -1;

    if(export(file, fp, CONFIG_OP_BEGIN, NULL))
        return free(tmp), fclose(fp), -1;

    for(opt = 0; opt < options.size; opt++)
    {
        if(export(file, fp, CONFIG_OP_OPTION, &options.list[opt]))
            return free(tmp), fclose(fp), -1;
    }

    if(export(file, fp, CONFIG_OP_END, NULL))
        return free(tmp), fclose(fp), -1;

    fclose(fp);

    if(rename(tmp, file))
    {
        fprintf(stderr, "%s: failed to replace: %s\n", file, strerror(errno));
        unlink(tmp);
        free(tmp);
        return -1;
    }

    free(tmp);

    return 0;
}

/// Interactive config modification.
///
/// \retval 0   success
/// \retval -1  error
static int config_modify(void)
{
    config_option_st *option;
    size_t opt, size = 0;
    char *line = NULL;
    ssize_t count;
    bool all = false;

    for(opt = 0, option = options.list; opt < options.size; opt++, option++)
    {
        if(!config_option_is_active(option))
            continue;

        printf("\n");
        config_option_print(option, 2);
        printf("\n");

        while(1)
        {
            printf("  [k,a,r,e,?] ");
            fflush(stdout);

            if((count = getline(&line, &size, stdin)) <= 0)
                return -1;

            switch(trim(line)[0])
            {
            case '?':
            case 'h':
                printf("  k(eep)  - keep value\n");
                printf("  a(ll)   - keep all values\n");
                printf("  q(uit)  - keep all values\n");
                printf("  r(eset) - reset to default value\n");
                printf("  e(dit)  - edit value\n");
                printf("  s(et)   - edit value\n");
                continue;

            case 'a':
            case 'q':
                all = true;
                break;

            case '\0':
            case 'k':
                break;

            case 'r':
                free(option->value);
                option->value = NULL;
                break;

            case 'e':
            case 's':

                while(1)
                {
                    printf("  %s = ", option->name);
                    fflush(stdout);

                    if((count = getline(&line, &size, stdin)) <= 0)
                        return -1;

                    for(; count && line[count - 1] == '\n'; count--)
                        line[count - 1] = '\0';

                    switch(config_option_set_value(option, line, false))
                    {
                    case 0:
                        break;

                    case E_CONFIG_INVALID_VALUE:
                        printf("  invalid value\n");
                        continue;

                    default:
                        return -1;
                    }

                    break;
                }

                break;

            default:
                printf("  invalid operation\n");
                continue;
            }

            break;
        }

        if(all)
            break;
    }

    free(line);

    return 0;
}

/// Print app usage.
///
/// \param name     app name
static void config_usage(const char *name)
{
    fprintf(stderr, "Usage: %s [-v] [-i] <template.cfg> <config.cfg> <header.h>\n", name);
}

/// config main
int main(int argc, char *argv[])
{
    const char *template, *config, *header;
    int opt;

    while((opt = getopt(argc, argv, "vi")) != -1)
    {
        switch(opt)
        {
        case 'v':
            verbose = true;
            break;

        case 'i':
            interactive = true;
            break;

        default:
            return config_usage(argv[0]), -1;
        }
    }

    if(argc - optind != 3)
        return config_usage(argv[0]), -1;

    template    = argv[optind++];
    config      = argv[optind++];
    header      = argv[optind++];

    if(verbose || interactive)
        printf("Config [%s|%s] -> %s\n", template, config, header);

    if(config_import(template, false, config_import_template))
        return config_options_free(), -1;

    if(config_import(config, true, config_import_config))
        return config_options_free(), -1;

    if(interactive && config_modify())
        return config_options_free(), -1;

    if(config_export(config, config_export_config))
        return config_options_free(), -1;

    if(config_export(header, config_export_header))
        return config_options_free(), -1;

    config_options_free();

    return 0;
}
