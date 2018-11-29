#include "fastspar_opts.h"


void print_help() {
    fprintf(stderr, "Program: FastSpar (c++ implementation of SparCC)\n");
    fprintf(stderr, "Version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "Contact: Stephen Watts (s.watts2@student.unimelb.edu.au)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  fastspar [options] --otu_table <path> --correlation <path> --covariance <path>\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -c <path>, --otu_table <path>\n");
    fprintf(stderr, "                OTU input OTU count table\n");
    fprintf(stderr, "  -r <path>, -correlation <path>\n");
    fprintf(stderr, "                Correlation output table\n");
    fprintf(stderr, "  -a <path>, --covariance <path>\n");
    fprintf(stderr, "                Covariance output table\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -i <int>, --iterations <int>\n");
    fprintf(stderr, "                Number of interations to perform (default: 50)\n");
    fprintf(stderr, "  -x <int>, --exclusion_iterations <int>\n");
    fprintf(stderr, "                Number of exclusion interations to perform (default: 10)\n");
    fprintf(stderr, "  -e <float>, --threshold <float>\n");
    fprintf(stderr, "                Correlation strength exclusion threshold (default: 0.1)\n");
    fprintf(stderr, "  -t <int>, --threads <int>\n");
    fprintf(stderr, "                Number of threads (default: 1)\n");
    fprintf(stderr, "  -s <int>, --seed <int>\n");
    fprintf(stderr, "                Random number generator seed (default: 1)\n");
    fprintf(stderr, "  -y, --yes\n");
    fprintf(stderr, "                Assume yes for prompts (default: unset)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Other:\n");
    fprintf(stderr, "  -h        --help\n");
    fprintf(stderr, "                Display this help and exit\n");
    fprintf(stderr, "  -v        --version\n");
    fprintf(stderr, "                Display version information and exit\n");
}


void print_version() {
    fprintf(stderr, "Program: FastSpar (c++ implementation of SparCC)\n");
    fprintf(stderr, "Version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "Contact: Stephen Watts (s.watts2@student.unimelb.edu.au)\n");
}


FastsparOptions get_commandline_arguments(int argc, char **argv) {
    // Get instance of FastsparOptions
    FastsparOptions fastspar_options;

    // Commandline arguments (for getlongtops)
    struct option long_options[] =
        {
            {"otu_table", required_argument, NULL, 'c'},
            {"correlation", required_argument, NULL, 'r'},
            {"covariance", required_argument, NULL, 'a'},
            {"iterations", required_argument, NULL, 'i'},
            {"exclude_iterations", required_argument, NULL, 'x'},
            {"threshold", required_argument, NULL, 'e'},
            {"threads", required_argument, NULL, 't'},
            {"seed", required_argument, NULL, 's'},
            {"yes", no_argument, NULL, 'y'},
            {"version", no_argument, NULL, 'v'},
            {"help", no_argument, NULL, 'h'},
            {NULL, 0, 0, 0}
        };

    // Parse commandline arguments
    while (1) {
        // Parser variables
        int option_index = 0;
        int c;

        // Parser
        c = getopt_long(argc, argv, "hvc:r:a:i:x:e:t:s:y", long_options, &option_index);

        // If no more arguments to parse, break
        if (c == -1) {
            break;
        }

        // Process current arguments
        switch(c) {
            case 'c':
                fastspar_options.otu_filename = optarg;
                break;
            case 'r':
                fastspar_options.correlation_filename = optarg;
                break;
            case 'a':
                fastspar_options.covariance_filename = optarg;
                break;
            case 'i':
                fastspar_options.iterations = int_from_optarg(optarg);
                break;
            case 'x':
                fastspar_options.exclude_iterations = int_from_optarg(optarg);
                break;
            case 'e':
                fastspar_options.threshold = float_from_optarg(optarg);
                break;
            case 't':
                fastspar_options.threads = int_from_optarg(optarg);
                break;
            case 's':
                fastspar_options.seed = int_from_optarg(optarg);
                break;
            case 'y':
                fastspar_options.assume_yes = true;
                break;
            case 'v':
                print_version();
                exit(0);
            case 'h':
                print_help();
                exit(0);
            default:
                exit(1);
        }
    }


    // Check if have an attempt at arguments
    if (argc < 7) {
        print_help();
        fprintf(stderr,"\n%s: error: option -c/--otu_table, -r/--correlation, and -a/--covariance are required\n", argv[0]);
        exit(1);
    }

    // Abort execution if given unknown arguments
    if (optind < argc){
        print_help();
        fprintf(stderr, "\n%s: invalid argument: %s\n", argv[0], argv[optind++]);
    }


    // Make sure we have filenames
    if (fastspar_options.otu_filename.empty()) {
        print_help();
        fprintf(stderr,"\n%s: error: argument -c/--otu_table is required\n", argv[0]);
        exit(1);
    }
    if (fastspar_options.correlation_filename.empty()) {
        print_help();
        fprintf(stderr,"\n%s: error: argument -r/--correlation is required\n", argv[0]);
        exit(1);
    }
    if (fastspar_options.covariance_filename.empty()) {
        print_help();
        fprintf(stderr,"\n%s: error: argument -a/--covariance is required\n", argv[0]);
        exit(1);
    }


    // Ensure threshold is less than 100
    if (fastspar_options.threshold > 1) {
        print_help();
        fprintf(stderr,"\n%s: error: exclusion threshold cannot be greater than 1.0\n", argv[0]);
        exit(1);
    }


    // Check if have a reasonable number of threads
    if (fastspar_options.threads < 1) {
        print_help();
        fprintf(stderr,"\n%s: error: must specify at least 1 thread\n", argv[0]);
        exit(1);
    }

    // Check we don't attempt to use more threads than we have
    unsigned int available_threads = std::thread::hardware_concurrency();
    if (available_threads > 1 && fastspar_options.threads > available_threads) {
        print_help();
        fprintf(stderr, "\n%s: error: only %d threads are available\n", argv[0], available_threads);
        exit(1);
    } else if (fastspar_options.threads > 64) {
        print_help();
        fprintf(stderr, "\n%s: error: current hardcode limit of 64 threads\n", argv[0]);
        exit(1);
    }


    // Check that the OTU file exists
    std::ifstream otu_file;
    otu_file.open(fastspar_options.otu_filename);
    if (!otu_file.good()) {
        print_help();
        fprintf(stderr, "\n%s: error: OTU table %s does not exist\n", argv[0], fastspar_options.otu_filename.c_str());
        exit(1);
    }


    return fastspar_options;
}
