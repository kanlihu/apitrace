/**************************************************************************
 *
 * Copyright 2010 VMware, Inc.
 * Copyright 2011 Intel corporation
 * All Rights Reserved.
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
 *
 **************************************************************************/

#include <set>
#include <sstream>
#include <string.h>
#include <limits.h> // for CHAR_MAX
#include <getopt.h>

#include "cli.hpp"

#include "os_string.hpp"

#include "trace_callset.hpp"
#include "trace_parser.hpp"
#include "trace_writer.hpp"

static const char *synopsis = "Create a new trace by merging multiple traces.";

static void
usage(void)
{
    std::cout
        << "usage: apitrace merge [OPTIONS] TRACE_FILE...\n"
        << synopsis << "\n"
        "\n"
        "    -h, --help               Show detailed help for merge options and exit\n"
        "    -o, --output=TRACE_FILE  Output trace file\n"
    ;
}

const static char *
shortOptions = "ho:";

const static struct option
longOptions[] = {
    {"help", no_argument, 0, 'h'},
    {"output", required_argument, 0, 'o'},
    {0, 0, 0, 0}
};

struct stringCompare {
    bool operator() (const char *a, const char *b) const {
        return strcmp(a, b) < 0;
    }
};

struct merge_options {
    /* Output filename */
    std::string output;
};

static int
merge_trace(int argc, char *argv[], struct merge_options *options)
{
    unsigned frame = 0;

    /* Prepare output file and writer for output. */
    if (options->output.empty()) {
        os::String base("output");
        base.trimExtension();

        options->output = std::string(base.str()) + std::string("-merge.trace");
    }

    trace::Writer writer;
    if (!writer.open(options->output.c_str())) {
        std::cerr << "error: failed to create " << options->output << "\n";
        return 1;
    }

	for (int i = 0; i < argc; i++ ) {
		trace::Parser p;
		char *filename = argv[i];
		if (!p.open(filename)) {
			std::cerr << "error: failed to open " << filename << "\n";
			return 1;
		}

		trace::Call *call;
		while ((call = p.parse_call())) {

			writer.writeCall(call);

			if (call->flags & trace::CALL_FLAG_END_FRAME) {
				frame++;
			}

			delete call;
		}
	}


    std::cerr << "merge trace is available as " << options->output << "\n";

    return 0;
}

static int
command(int argc, char *argv[])
{
    struct merge_options options;

    int opt;
    while ((opt = getopt_long(argc, argv, shortOptions, longOptions, NULL)) != -1) {
        switch (opt) {
        case 'h':
            usage();
            return 0;
        case 'o':
            options.output = optarg;
            break;
        default:
            std::cerr << "error: unexpected option `" << (char)opt << "`\n";
            usage();
            return 1;
        }
    }

    if (argc < optind + 2) {
        std::cerr << "error: apitrace merge requires two trace file as an argument.\n";
        usage();
        return 1;
    }

    return merge_trace(argc - optind , &argv[optind], &options);
}

const Command merge_command = {
    "merge",
    synopsis,
    usage,
    command
};
