#include <cmath>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <iostream>
#include <string>
#include <ios>
#include <fstream>

struct Token // ���� ���������
{
	Token(char c) :
		kind(c)
	{}

	Token(char c, double v) :
		kind(c), value(v)
	{}

	Token(char c, std::string n) : // ��� ����������
		kind(c), name(n)
	{}

	char kind;
	double value;
	std::string name; // ��� ����������
};

template < typename S >
struct Token_Stream // ��� ��
{
	Token_Stream(S& stream) :
		m_full(false), m_buffer(0), m_stream(stream)
	{}

	Token get(Token_Stream& token_stream);

	void putback(const Token& token);

	bool m_full;

	Token m_buffer;

	S& m_stream;
};

struct Variable // �����
{
	Variable(std::string n, double v) :
		name(n), value(v)
	{}

	std::string name;
	double value;
};

template < typename S >
void calculate(Token_Stream < S >& token_stream, char quit, char print);
template < typename S >
double expression(Token_Stream < S >& token_stream);
template < typename S >
double term(Token_Stream < S >& token_stream);
template < typename S >
double statement(Token_Stream < S >& token_stream);
template < typename S >
double primary(Token_Stream < S >& token_stream);

// �����
double get_value(const std::string& n);
void set_value(const std::string& n, double v, std::vector < Variable > variables);
bool is_declared(const std::string& n);
double define_name(const std::string& n, double v);
template < typename S >
double declaration(Token_Stream < S >& token_stream);

int main()
{
	std::cout.precision(3);
	std::cout.setf(std::ios::fixed);

	//std::ifstream fin("test.txt", std::ios::in);

	//Token_Stream < std::ifstream > token_stream{ fin };
	Token_Stream < std::istream > token_stream{ std::cin };

	const char quit = 'q';
	const char print = ';';

	calculate(token_stream, quit, print);

	system("pause");

	return 0;

}

template < typename S >
void calculate(Token_Stream < S >& token_stream, char quit, char print)
{
	const std::string prompt = "> ";
	const std::string result = "= ";

	while (token_stream.m_stream)
	{
		std::cout << prompt;

		Token token = token_stream.get(token_stream);

		while (token.kind == print)
		{
			token = token_stream.get(token_stream);
		}
		if (token.kind == quit)
		{
			return;
		}

		token_stream.putback(token);

		std::cout << "\n" << result << statement(token_stream) << '\n'; // expression
	}
}

template < typename S >
void Token_Stream < S >::putback(const Token& token)
{
	m_buffer = token;
	m_full = true;
}

const char number = '8';

const char set = 'S'; // �����
const char name = 'a'; // ����� 
const std::string declkey = "let"; // �����

template < typename S >
Token Token_Stream < S >::get(Token_Stream < S >& token_stream)
{
	if (m_full)
	{
		m_full = false;
		return m_buffer;
	}
	char c;
	token_stream.m_stream >> c;
	switch (c)
	{
	case 'q': case ';': case '(': case ')': case '{':
	case '}': case '+': case '-': case '*': case '/':
	case '%': case '=': case '!': case 's': case 'p':
		return Token(c);
	case '.':
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	{
		token_stream.m_stream.putback(c);
		double value;
		token_stream.m_stream >> value;
		return Token(number, value);
	}
	default: // �����
		if (isalpha(c))
		{
			std::string s;
			s += c;
			while (token_stream.m_stream.get(c) && (isalpha(c) || isdigit(c)))
			{
				s += c;
			}
			token_stream.m_stream.putback(c);
			if (s == declkey)
			{
				return Token(set);
			}
			return Token(name, s);
		}
	}
}

template < typename S >
double primary(Token_Stream < S >& token_stream)
{
	Token t = token_stream.get(token_stream);
	switch (t.kind)
	{
	case name: // �����
		return get_value(t.name);
		break;
	case '(':
	{
		double d = expression(token_stream);
		t = token_stream.get(token_stream);
		return d;
		break;
	}
	case '{':
	{
		double d = expression(token_stream);
		t = token_stream.get(token_stream);
		return d;
	}
	case number:
		return t.value;
	case '-':
		return -primary(token_stream);
	case '+':
		return primary(token_stream);
	}
}

template < typename S >
double expression(Token_Stream  < S >& token_stream)
{
	double left = term(token_stream);
	Token t = token_stream.get(token_stream);
	while (true)
	{
		switch (t.kind)
		{
		case '+':
			left += term(token_stream);
			t = token_stream.get(token_stream);
			break;
		case '-':
			left -= term(token_stream);
			t = token_stream.get(token_stream);
			break;
		default:
			token_stream.putback(t);
			return left;
			break;
		}
	}
}

template < typename S >
double statement(Token_Stream < S >& token_stream) // �����
{
	Token token = token_stream.get(token_stream);
	switch (token.kind)
	{
	case set:
		return declaration(token_stream);
	default:
		token_stream.putback(token);
		return expression(token_stream);
	}
}

template < typename S >
double term(Token_Stream < S >& token_stream)
{
	double left = primary(token_stream);
	Token t = token_stream.get(token_stream);
	while (true)
	{
		switch (t.kind)
		{
		case 'p':
		{
			double power = primary(token_stream);
			left = pow(left, power);
			t = token_stream.get(token_stream);
			break;
		}
		case '!':
		{
			for (int i = static_cast <int> (left) - 1; i > 1; i--)
			{
				left *= i;
			}
			t = token_stream.get(token_stream);
			break;
		}
		case 's':
		{
			left = sqrt(left);
			t = token_stream.get(token_stream);
			break;
		}
		case '*':
			left *= primary(token_stream);
			t = token_stream.get(token_stream);
			break;
		case '/':
		{
			double d = primary(token_stream);
			left /= d;
			t = token_stream.get(token_stream);
			break;
		}
		case '%':
		{
			double d = primary(token_stream);
			left = fmod(left, d);
			t = token_stream.get(token_stream);
			break;
		}
		default:
			token_stream.putback(t);
			return left;
			break;
		}
	}
}

// �����

std::vector < Variable > variables;

double get_value(const std::string& n)
{
	for (const Variable& variable : variables)
	{
		if (variable.name == n)
		{
			return variable.value;
		}
	}
}

void set_value(const std::string& n, double v)
{
	for (Variable& variable : variables)
	{
		if (variable.name == n)
		{
			variable.value = v;
			return;
		}
	}
}

template < typename S >
double declaration(Token_Stream < S >& token_stream)
{
	Token token = token_stream.get(token_stream);
	std::string name = token.name;
	Token assume = token_stream.get(token_stream);
	double value = expression(token_stream);
	define_name(name, value);

	return value;
}

bool is_declared(const std::string& n)
{
	for (const Variable& variable : variables)
	{
		if (variable.name == n)
		{
			return true;
		}
	}
	return false;
}

double define_name(const std::string& n, double v)
{
	if (is_declared(n))
	{
		// error
	}
	variables.push_back(Variable(n, v));
	return v;
}
