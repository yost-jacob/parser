#include <iostream>
#include <charconv>
#include <locale>
#include "./jacob_parser.h"



int main() {

    std::string xml{"<?xml version='1.0' encoding='utf-8' standalone='no'?><!--ProLog Comment--><?PP prologpi?> <e12>abc0<![CDATA[ghj]]>abc</e12><  element attr1 =   \"dt&apos;1\" superlongattributename=\"d't'2 super long attribute well beyond the length of a small string\"  >  data  </element><e2 attr3=\'jacob&#x7EFF;yost\' attr4=\"amelia&#126;yost\">data2</e2><e3/><e4><e5>this is a very long child string well beyond the length of a small string</e5><e6>child2</e6></e4><!--e7--><?jacob bobbitybobbity boo?>"};
    xml_document<char> doc{};
    auto i = doc.parse(xml);

    std::cout << "exiting main" << std::endl;



    return i;
}
