#include "cli.hpp"

using namespace qOS;

bool commandLineInterface::setup( cli::putChar_t outFcn, char *pIntput, const size_t sizeInput, char *pOutput, const size_t sizeOutput )
{
    outputFcn = outFcn;
    this->sizeOutput = sizeOutput;

}