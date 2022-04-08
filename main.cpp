#include "Parser.hpp"


int main(int argc, char **argv)
{
    TS_Parser *parser = nullptr;

    int opt = getopt(argc, argv, "m:f:");
    switch (opt) {
    case 'm':
        parser = new ParserFromCast(optarg);
        break;
    case 'f':
        parser = new ParserFromFile(optarg);
        break;
    default:
        std::cerr << "Usage: [-mf] source\n";
        return 1;
    }

    bool ok = parser->Init();
    if (!ok)
        return 2;
    return parser->Parse();
}
