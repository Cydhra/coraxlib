#include "pll.h"
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

void delete_unode(pll_unode_t *node) {
  if (node->label) { free(node->label); }
  free(node);
}

inline void close_node_loop(pll_unode_t *start) {
  pll_unode_t *cur = start;
  while (cur->next != nullptr) { cur = cur->next; }
  cur->next = start;
}

inline void set_mutual_back_pointers(pll_unode_t *a, pll_unode_t *b) {
  a->back = b;
  b->back = a;
  if (a->length == 0.0) {
    a->length = b->length;
  } else {
    b->length = a->length;
  }
}

static void fill_nodes_recursive(pll_unode_t * node,
                                 pll_unode_t **array,
                                 unsigned int  array_size,
                                 unsigned int *tip_index,
                                 unsigned int *inner_index,
                                 unsigned int  level) {
  unsigned int index;
  if (!node->next) {
    /* tip node */
    index = *tip_index;
    *tip_index += 1;
  } else {
    /* inner node */
    pll_unode_t *snode = level ? node->next : node;
    do {
      fill_nodes_recursive(
          snode->back, array, array_size, tip_index, inner_index, level + 1);
      snode = snode->next;
    } while (snode != node);

    index = *inner_index;
    *inner_index += 1;
  }

  assert(index < array_size);
  array[index] = node;
}

pll_unode_t *trim_node(pll_unode_t *node) {
  pll_unode_t *next, *prev;
  next = node->next;
  prev = node;

  while (prev->next != node) { prev = prev->next; }

  prev->next = next;
  delete_unode(node);

  return prev;
}

pll_unode_t *unode_unroot(pll_unode_t *vroot) {
  if (vroot->next->next != vroot) { return vroot; }

  pll_unode_t *lchild, *rchild;
  lchild = vroot->back;
  rchild = vroot->next->back;

  double total_len = vroot->length + vroot->next->length;

  lchild->length = total_len;
  rchild->length = total_len;

  lchild->back = rchild;
  rchild->back = lchild;

  free(vroot->next);
  free(vroot);

  if (lchild->next != nullptr) { return lchild; }
  return rchild;
}

enum lexeme_type_t {
  OPENING_SQUARE_BRACKET,
  CLOSING_SQUARE_BRACKET,
  OPENING_PAREN,
  CLOSING_PAREN,
  COLON,
  SEMICOLON,
  COMMA,
  VALUE,
  END
};

class newick_lexer_t {
public:
  newick_lexer_t(std::string input) :
      _input{std::move(input)}, _current_index{0} {};

  lexeme_type_t consume();
  lexeme_type_t peak();

  std::string consume_value_as_string() {
    std::string tmp;
    std::swap(tmp, _value);
    return tmp;
  }

  /* WARNING, ALLOCATES MEMORY */
  char *consume_value_as_cstring() {
    char *label = (char *)calloc(
        sizeof(char),
        (_value.size() + 1) /* Need to include space for the null */
    );

    for (size_t i = 0; i < _value.size(); ++i) { label[i] = _value[i]; }
    _value.clear();
    return label;
  }

  double consume_value_as_float() {
    auto   f_str = consume_value_as_string();
    size_t pos   = 0;
    double val   = std::stod(f_str, &pos);
    if (pos != f_str.size()) {
      throw std::runtime_error{std::string("Float conversion failed around") +
                               describe_position()};
    }
    return val;
  }

  std::string describe_position() const {
    std::stringstream builder;
    builder << "position " << _current_index;
    return builder.str();
  }

  void expect(lexeme_type_t token_type) {
    auto ret = consume_token_pos();
    if (ret.first != token_type) {
      throw std::runtime_error{
          std::string("Got the wrong token type at position ") +
          std::to_string(ret.second + 1) + " was expecting " +
          describe_token(token_type)};
    }
  }

  void consume_until(lexeme_type_t token_type) {
    while (token_type != consume()) {}
  }

  bool at_end() { return _input.size() == _current_index; }

private:
  bool is_punct(char c) {
    return c == '[' || c == ']' || c == '(' || c == ')' || c == ':' ||
           c == ';' || c == ',' || c == 0 || c == EOF;
  }

  std::pair<lexeme_type_t, size_t> consume_token_pos() {
    auto start_index = _current_index;
    auto token       = consume();
    return {token, start_index};
  }

  std::string describe_token(lexeme_type_t token_type) {
    switch (token_type) {
    case OPENING_SQUARE_BRACKET:
      return {"opening square bracket"};
    case CLOSING_SQUARE_BRACKET:
      return {"closing square bracket"};
    case OPENING_PAREN:
      return {"opening parenthesis"};
    case CLOSING_PAREN:
      return {"closing parenthesis"};
    case COLON:
      return {"colon"};
    case SEMICOLON:
      return {"semicolon"};
    case COMMA:
      return {"comma"};
    case END:
      return {"end of input"};
    case VALUE:
      return {"either a identifier or a number"};
    default:
      return {"unknown token"};
    }
  }

  void skip_whitespace() {
    while (_current_index < _input.size()) {
      char c = _input[_current_index];
      if (!std::isspace(c)) { break; }
      _current_index++;
    }
  }

  std::string _input;
  std::string _value;
  size_t      _current_index;
};

lexeme_type_t newick_lexer_t::peak() {
  size_t tmp_index    = _current_index;
  char   current_char = _input[tmp_index++];
  if (is_punct(current_char)) {
    switch (current_char) {
    case '[':
      return OPENING_SQUARE_BRACKET;
    case ']':
      return CLOSING_SQUARE_BRACKET;
    case '(':
      return OPENING_PAREN;
    case ')':
      return CLOSING_PAREN;
    case ':':
      return COLON;
    case ';':
      return SEMICOLON;
    case ',':
      return COMMA;
    case 0:
    case EOF:
      return END;
    default:
      throw std::runtime_error{"The punctuation was unrecognized"};
    }
  } else {
    return VALUE;
  }
}

lexeme_type_t newick_lexer_t::consume() {
  auto token = peak();
  if (token == VALUE) {
    // we have a value, so we need to scan until we have found punctuation, or
    // the end of the string
    std::stringstream builder;
    while (char tmp = _input[_current_index]) {
      if (is_punct(tmp)) { break; }
      builder << tmp;
      _current_index++;
    }

    _value = builder.str();
    while (std::isspace(*(_value.end() - 1))) {
      _value.resize(_value.size() - 1);
    }
    return token;
  } else {
    _current_index++;
    skip_whitespace();
    return token;
  }
}

/* Using the following grammar:
 * <tree> ::=
 *     <subtree> ";"
 * <subtree> ::=
 *     <leaf> |
 *     <internal>
 * <internal> ::=
 *     "(" <node_set> ")" <node_attrs>
 * <node_set> ::=
 *     <node> |
 *     <node> "," <node_set>
 * <node> ::=
 *     <subtree> <length>
 * <node_attrs> ::=
 *     <name> <length> <comment>
 * <leaf> ::=
 *     <node_attrs>
 * <length> ::=
 *     ":" <number> |
 *     <empty>
 * <name> ::=
 *     <string> |
 *     <empty>
 * <string> ::=
 *     anything but punctuation
 * <number> ::=
 *     [-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?
 * <comment> ::=
 *     "[" .* "]" |
 *     <empty>
 * <empty> ::=
 *     ""
 */
class newick_parser_t {
public:
  newick_parser_t(std::string input) :
      _lexer{std::move(input)},
      _tip_count{0},
      _inner_count{0},
      _edge_count{0} {};
  pll_utree_t *parse() { return parse_utree(); }

private:
  pll_utree_t *parse_utree();
  pll_unode_t *parse_subtree();
  pll_unode_t *parse_internal(); // creates node
  pll_unode_t *parse_node_set();
  void         parse_node_attrs(pll_unode_t *current_node);
  pll_unode_t *parse_leaf(); // creates node
  void         parse_length(pll_unode_t *current_node);
  void         parse_name(pll_unode_t *current_node);
  std::string  parse_string();
  char *       parse_cstring();
  double       parse_number();
  void         parse_comment();

  /* member variables */
  newick_lexer_t _lexer;
  size_t         _tip_count;
  size_t         _inner_count;
  size_t         _edge_count;
};

pll_utree_t *newick_parser_t::parse_utree() {
  auto root_node = parse_subtree();
  root_node      = trim_node(root_node);
  /* We overcounted, because we assume there is an "upper" branch still */
  _edge_count--;
  auto new_root_node = unode_unroot(root_node);
  if (new_root_node != root_node) {
    _inner_count--;
    _edge_count--;
    root_node = new_root_node;
  }
  _lexer.expect(SEMICOLON);
  if (!_lexer.at_end()) {
    throw std::runtime_error{
        "There were extra charcters when we finished parsing"};
  }

  auto current_tree         = (pll_utree_t *)calloc(sizeof(pll_utree_t), 1);
  current_tree->tip_count   = _tip_count;
  current_tree->inner_count = _inner_count;
  current_tree->edge_count  = _edge_count;
  size_t node_array_size    = _tip_count + _inner_count;
  current_tree->nodes =
      (pll_unode_t **)malloc(sizeof(pll_unode_t *) * node_array_size);

  unsigned int tip_index   = 0;
  unsigned int inner_index = _tip_count;
  fill_nodes_recursive(root_node,
                       current_tree->nodes,
                       node_array_size,
                       &tip_index,
                       &inner_index,
                       /*level=*/0);

  current_tree->vroot = root_node;
  return current_tree;
}

pll_unode_t *newick_parser_t::parse_subtree() {
  auto token = _lexer.peak();
  _edge_count++;

  if (token == OPENING_PAREN) {
    auto tmp = parse_internal();
    /* error checking on tmp here */
    return tmp;
  } else {
    auto tmp = parse_leaf();
    /* error checking on tmp here */
    return tmp;
  }
}

pll_unode_t *newick_parser_t::parse_internal() {
  _lexer.expect(OPENING_PAREN);

  auto extra_node   = (pll_unode_t *)calloc(sizeof(pll_unode_t), 1);
  auto current_node = parse_node_set();
  extra_node->next  = current_node;
  close_node_loop(extra_node);

  _lexer.expect(CLOSING_PAREN);

  parse_node_attrs(extra_node);

  _inner_count++;

  return extra_node;
}

pll_unode_t *newick_parser_t::parse_node_set() {
  auto current_node = (pll_unode_t *)calloc(sizeof(pll_unode_t), 1);
  auto child        = parse_subtree();
  set_mutual_back_pointers(current_node, child);
  auto token = _lexer.peak();
  if (token == COMMA) {
    _lexer.consume();
    auto next          = parse_node_set();
    current_node->next = next;
  }
  return current_node;
}

void newick_parser_t::parse_node_attrs(pll_unode_t *current_node) {
  parse_name(current_node);
  parse_length(current_node);
  parse_comment();
}

pll_unode_t *newick_parser_t::parse_leaf() {
  // auto current_node = std::make_shared<Node>();
  auto current_node = (pll_unode_t *)calloc(sizeof(pll_unode_t), 1);
  parse_node_attrs(current_node);
  if (current_node->label == nullptr) {
    throw std::runtime_error{
        std::string("Got a leaf with an empty name around ") +
        std::string(_lexer.describe_position())};
  }
  _tip_count++;
  return current_node;
}

void newick_parser_t::parse_length(pll_unode_t *current_node) {
  auto token = _lexer.peak();
  if (token == COLON) {
    _lexer.consume();
    _lexer.expect(VALUE);
    current_node->length = parse_number();
  }
}

void newick_parser_t::parse_name(pll_unode_t *current_node) {
  auto token = _lexer.peak();
  if (token == VALUE) {
    _lexer.consume();
    current_node->label = parse_cstring();
  }
}

std::string newick_parser_t::parse_string() {
  return _lexer.consume_value_as_string();
}

char *newick_parser_t::parse_cstring() {
  return _lexer.consume_value_as_cstring();
}

double newick_parser_t::parse_number() {
  return _lexer.consume_value_as_float();
}

void newick_parser_t::parse_comment() {
  auto token = _lexer.peak();
  if (token == OPENING_SQUARE_BRACKET) {
    _lexer.consume();
    _lexer.consume_until(CLOSING_SQUARE_BRACKET);
  }
};

PLL_EXPORT pll_utree_t *pll_utree_parse_newick(const char *filename) {
  std::ifstream   newick_file(filename);
  std::string     newick_string((std::istreambuf_iterator<char>(newick_file)),
                            (std::istreambuf_iterator<char>()));
  newick_parser_t np(newick_string);
  return np.parse();
}

pll_utree_t *pll_utree_parse_newick_string(const char *newick_cstring) {
  std::string     newick_string(newick_cstring);
  newick_parser_t np(newick_string);
  return np.parse();
}
