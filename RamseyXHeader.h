#ifndef RX_RAMSEYXHEADER_H
#define RX_RAMSEYXHEADER_H

#include <ctime>
#include <atomic>
#include <bitset>
#include <cstring>
#include <fstream>

// Version
const int RX_VER_MAJOR = 5;
const int RX_VER_MINOR = 0;
const int RX_VER_PATCHLEVEL = 2;

// Data
const unsigned int RX_N = 3;
const unsigned int RX_P = 67;
const unsigned int RX_MN = 33;
const unsigned int RX_G = 2;
const unsigned int RX_Q1 = 3;
const unsigned int RX_Q2 = 3;
const unsigned int RX_Q3 = 6;

const unsigned long long RX_APPROXIMATE_BLOCK_LENGTH = 50000000ULL;
const unsigned int RX_LAYER1_BLOCKS_PER_TASK = 1U << 17; // 131072

const int RX_MAX_DAYS = 5;

// Error codes
const int RX_ERR_SUCCESS = 0x0000;
const int RX_ERR_INVALID_ARGUMENTS = 0x0001;
const int RX_ERR_CONNECTION_FAILED = 0x0002;
const int RX_ERR_LOGIN_FAILED = 0x0003;
const int RX_ERR_WRONG_USR_PWD = 0x0004;
const int RX_ERR_GET_FAILED = 0x0005;
const int RX_ERR_NO_NEW_TASK = 0x0006;
const int RX_ERR_UPLOAD_FAILED = 0x0007;
const int RX_ERR_USER_INFO_UNAVAILABLE = 0x0008;
const int RX_ERR_USER_ALREADY_EXIST = 0x0009;
const int RX_ERR_EMAIL_ALREADY_EXIST = 0x000a;
const int RX_ERR_INVALID_RECOMMENDER = 0x000b;
const int RX_ERR_SIGNUP_FAILED = 0x000c;
const int RX_ERR_TASK_OUTDATED = 0x000d;
const int RX_ERR_TASK_LISTS_FULL = 0x000e;
const int RX_ERR_NO_COMPLETED_TASK = 0x0010;

const int RX_MAX_TASK_NUM_TIMES = 2;

const int RX_TASKINFO_BLOCKNUM = 0;
const int RX_TASKINFO_W1SIZE = 1;
const int RX_TASKINFO_BLOCKLENGTH = 2;

const int RX_LAUNCH_INCOMPLETE = -1;
const int RX_LAUNCH_SUCCESS = 0;
const int RX_LAUNCH_REACHED_BLOCK_END = 1;
const int RX_LAUNCH_INVALID_ARGUMENTS = 2;

const int RX_UNNECESSARY = 0;
const int RX_OPTIONAL = 1;
const int RX_NECESSARY = 2;

const unsigned int RX_UPDATE_PROGRESS_COUNT = (1U << 20) - 1;

const unsigned long long taskInfo[][3] = {
    {1, 43108065, 50019606},
    {5, 215540325, 50114610},
    {18, 862161300, 50352120},
    {55, 2832815700, 51505740},
    {152, 7790243175, 51505740},
    {363, 18177234075, 50075025},
    {605, 36354468150, 60090030},
    {908, 62794081350, 69194580},
    {1815, 94191122025, 51895935},
    {1815, 123173005725, 67863915},
    {1815, 140769149400, 77558760},
    {1815, 140769149400, 77558760},
    {1815, 123173005725, 67863915},
    {1815, 94191122025, 51895935},
    {908, 62794081350, 69194580},
    {605, 36354468150, 60090030},
    {363, 18177234075, 50075025},
    {152, 7790243175, 51505740},
    {55, 2832815700, 51505740},
    {5, 221457600, 50020425},
    {22, 1062996480, 50024520},
    {82, 4074819840, 50106420},
    {252, 12806576640, 50913720},
    {637, 33617263680, 52837785},
    {1352, 74705030400, 55255200},
    {2704, 141939557760, 52492440},
    {3606, 232264730880, 64422540},
    {5408, 329041702080, 60843510},
    {5408, 404974402560, 74884320},
    {5408, 433901145600, 80233200},
    {5408, 404974402560, 74884320},
    {5408, 329041702080, 60843510},
    {3606, 232264730880, 64422540},
    {2704, 141939557760, 52492440},
    {1352, 74705030400, 55255200},
    {637, 33617263680, 52837785},
    {252, 12806576640, 50913720},
    {20, 957615750, 50017500},
    {89, 4405032450, 50052600},
    {323, 16151785650, 50025690},
    {958, 48455356950, 50617710},
    {2373, 121138392375, 51061725},
    {4961, 255736606125, 51555075},
    {9095, 460325891025, 50617710},
    {13642, 711412740675, 52151580},
    {18189, 948550320900, 52151580},
    {18189, 1094481139500, 60174900},
    {18189, 1094481139500, 60174900},
    {18189, 948550320900, 52151580},
    {13642, 711412740675, 52151580},
    {9095, 460325891025, 50617710},
    {4961, 255736606125, 51555075},
    {2373, 121138392375, 51061725},
    {958, 48455356950, 50617710},
    {71, 3527856150, 50007750},
    {311, 15522567060, 50058580},
    {1083, 54328984710, 50190140},
    {3065, 155225670600, 50650600},
    {7151, 368660967675, 51555075},
    {13881, 737321935350, 53117350},
    {23598, 1253447290095, 53117350},
    {33711, 1823196058320, 54083120},
    {39330, 2278995072900, 57946200},
    {47196, 2454302386200, 52003000},
    {39330, 2278995072900, 57946200},
    {33711, 1823196058320, 54083120},
    {23598, 1253447290095, 53117350},
    {13881, 737321935350, 53117350},
    {7151, 368660967675, 51555075},
    {3065, 155225670600, 50650600},
    {224, 11175806950, 50005450},
    {938, 46938389190, 50048460},
    {3122, 156461297300, 50119300},
    {8414, 424680664100, 50473500},
    {18798, 955531494225, 50834025},
    {35339, 1804892822425, 51074375},
    {55217, 2887828515880, 52300160},
    {73622, 3937947976200, 53488800},
    {88347, 4594272638900, 52003000},
    {88347, 4594272638900, 52003000},
    {73622, 3937947976200, 53488800},
    {55217, 2887828515880, 52300160},
    {35339, 1804892822425, 51074375},
    {18798, 955531494225, 50834025},
    {8414, 424680664100, 50473500},
    {613, 30615291168, 50005956},
    {2448, 122461164672, 50027208},
    {7746, 387793688128, 50069712},
    {19871, 997183769472, 50185080},
    {42371, 2119015510128, 50012028},
    {73877, 3767138684672, 50992656},
    {110815, 5650708027008, 50992656},
    {137199, 7191810216192, 52419024},
    {151641, 7791127734208, 51378964},
    {137199, 7191810216192, 52419024},
    {110815, 5650708027008, 50992656},
    {73877, 3767138684672, 50992656},
    {42371, 2119015510128, 50012028},
    {19871, 997183769472, 50185080},
    {1454, 72697619610, 50004185},
    {5525, 276250954518, 50002414},
    {16552, 828752863554, 50069712},
    {40245, 2012685525774, 50012028},
    {80489, 4025371051548, 50012028},
    {132416, 6708951752580, 50665780},
    {186586, 9392532453612, 50338904},
    {221886, 11100265626996, 50026886},
    {221886, 11100265626996, 50026886},
    {186586, 9392532453612, 50338904},
    {132416, 6708951752580, 50665780},
    {80489, 4025371051548, 50012028},
    {40245, 2012685525774, 50012028},
    {2993, 149621448900, 50005340},
    {10771, 538637216040, 50008266},
    {30483, 1526138778780, 50065323},
    {69572, 3488317208640, 50139936},
    {130281, 6540594766200, 50203890},
    {202516, 10174258525200, 50239420},
    {262232, 13226536082760, 50438388},
    {288086, 14428948453920, 50085672},
    {262232, 13226536082760, 50438388},
    {202516, 10174258525200, 50239420},
    {130281, 6540594766200, 50203890},
    {69572, 3488317208640, 50139936},
    {5331, 266553472185, 50004675},
    {18120, 906281805429, 50017842},
    {48305, 2416751481144, 50031408},
    {103575, 5178753173880, 50000400},
    {181045, 9062818054290, 50058540},
    {260450, 13090737189530, 50262030},
    {313641, 15708884627436, 50085672},
    {313641, 15708884627436, 50085672},
    {260450, 13090737189530, 50262030},
    {181045, 9062818054290, 50058540},
    {103575, 5178753173880, 50000400},
    {8213, 410603560965, 50000400},
    {26279, 1313931395088, 50000400},
    {65697, 3284828487720, 50000400},
    {131393, 6569656975440, 50000400},
    {213471, 10675692585090, 50010090},
    {284389, 14234256780120, 50052080},
    {312723, 15657682458132, 50068876},
    {284389, 14234256780120, 50052080},
    {213471, 10675692585090, 50010090},
    {131393, 6569656975440, 50000400},
    {10942, 547055108964, 50000400},
    {32824, 1641165326892, 50000400},
    {76582, 3829385762748, 50004276},
    {142135, 7111716416532, 50035284},
    {213202, 10667574624798, 50035284},
    {260405, 13038146763642, 50068876},
    {260405, 13038146763642, 50068876},
    {213202, 10667574624798, 50035284},
    {142135, 7111716416532, 50035284},
    {12623, 631141692300, 50000400},
    {35342, 1767196738440, 50002848},
    {76562, 3828926266620, 50011416},
    {131206, 6563873599920, 50027328},
    {180451, 9025326199890, 50015394},
    {200443, 10028140222100, 50029980},
    {180451, 9025326199890, 50015394},
    {131206, 6563873599920, 50027328},
    {12623, 631134695100, 50001420},
    {32816, 1640950207260, 50005228},
    {65624, 3281900414520, 50011416},
    {103144, 5157272079960, 50000808},
    {128918, 6446590099950, 50005670},
    {128918, 6446590099950, 50005670},
    {103144, 5157272079960, 50000808},
    {10940, 546983156720, 50000860},
    {26255, 1312759576128, 50000496},
    {48133, 2406725889568, 50001952},
    {68758, 3438179842240, 50004240},
    {77340, 3867952322520, 50012820},
    {68758, 3438179842240, 50004240},
    {8205, 410237366175, 50001315},
    {18050, 902522205585, 50002953},
    {30082, 1504203675975, 50004955},
    {38675, 1933976154825, 50006385},
    {38675, 1933976154825, 50006385},
    {5309, 265447707525, 50000951},
    {10618, 530895415050, 50001952},
    {15926, 796343122575, 50002953},
    {18202, 910106425800, 50000808},
    {2950, 147470948625, 50000665},
    {5309, 265447707525, 50001237},
    {7079, 353930276700, 50000808},
    {1398, 69854659875, 50000445},
    {2236, 111767455800, 50000544},
    {559, 27941863950, 50000280}
};

struct RXFLAG
{
    std::atomic<bool> useFlag{false};
    std::atomic<bool> termFlag{false};
};

struct RXTASKINFO
{
    unsigned long long id = 0;
    unsigned int layer = 0;
    std::time_t deadline = 0;
    unsigned long long combinationNum = 0;
    unsigned long long block = 0;
    unsigned long long offset = 0;
    unsigned long long complexity = 0;
    int result = RX_LAUNCH_INCOMPLETE;
    unsigned int threadID = 0;

    std::bitset<RX_LAYER1_BLOCKS_PER_TASK> resultBits;
};

inline bool operator==(const RXTASKINFO &lhs, const RXTASKINFO &rhs)
{
    static const std::size_t builtInPartSize =
            reinterpret_cast<const char *>(&lhs.resultBits) - reinterpret_cast<const char *>(&lhs);

    return !std::memcmp(&lhs, &rhs, builtInPartSize) && lhs.resultBits == rhs.resultBits;
}

inline std::ofstream &operator<<(std::ofstream &lhs, const RXTASKINFO &rhs)
{
    static const std::size_t builtInPartSize =
            reinterpret_cast<const char *>(&rhs.resultBits) - reinterpret_cast<const char *>(&rhs);
    static const std::size_t bitsetSize =
            sizeof (rhs.resultBits);

    /*lhs.write(reinterpret_cast<const char *>(&rhs), builtInPartSize);
    if (!lhs)
        return lhs;

    const unsigned int cnt = rhs.resultBits.count();
    lhs.write(reinterpret_cast<const char *>(&cnt), sizeof (cnt));
    for (unsigned int i = 0; lhs && i < RX_LAYER1_BLOCKS_PER_TASK; ++i)
        if (rhs.resultBits[i])
            lhs.write(reinterpret_cast<const char *>(&i), sizeof (i));*/
    lhs.write(reinterpret_cast<const char *>(&rhs), builtInPartSize + bitsetSize);

    return lhs;
}

inline std::ifstream &operator>>(std::ifstream &lhs, RXTASKINFO &rhs)
{
    static const std::size_t builtInPartSize =
            reinterpret_cast<const char *>(&rhs.resultBits) - reinterpret_cast<const char *>(&rhs);
    static const std::size_t bitsetSize =
            sizeof (rhs.resultBits);

    /*if (builtInPartSize != lhs.read(reinterpret_cast<char *>(&rhs), builtInPartSize).gcount())
        return lhs;

    unsigned int cnt = 0;
    if (sizeof (cnt) != lhs.read(reinterpret_cast<char *>(&cnt), sizeof (cnt)).gcount())
        return lhs;
    while (cnt--)
    {
        unsigned int temp = 0;
        if (sizeof (temp) != lhs.read(reinterpret_cast<char *>(&temp), sizeof (temp)).gcount())
            return lhs;
        rhs.resultBits.set(temp);
    }*/

    /*if (builtInPartSize + bitsetSize != lhs.read(reinterpret_cast<char *>(&rhs), builtInPartSize + bitsetSize).gcount())
    {
        lhs.clear(std::ios_base::failbit);
        return lhs;
    }*/

    lhs.read(reinterpret_cast<char *>(&rhs), builtInPartSize + bitsetSize);

    return lhs;
}

struct RXPRINT
{
    unsigned long long id = 0;
    unsigned int layer = 0;
    std::time_t deadline = 0;
    double progress = 0.0;

    explicit RXPRINT(const RXTASKINFO &info) :
        id (info.id),
        layer(info.layer),
        deadline(info.deadline),
        progress(info.layer == 1 ?
                     100.0 * info.offset / RX_LAYER1_BLOCKS_PER_TASK :
                     100.0 * info.offset / taskInfo[info.combinationNum][RX_TASKINFO_BLOCKLENGTH])
    { }
};

// Determine OS
#if defined(_WIN64)
    #define RX_OS "win64"
#elif defined(_WIN32)
    #define RX_OS "win32"
#elif defined(__linux__)
    #define RX_OS "linux"
#elif defined(__APPLE__) && defined(__MACH__)
    #define RX_OS "apple"
#else
    #error Unsupported OS
#endif

// Determine compiler
#if defined(__clang__)
    /* Clang/LLVM. ---------------------------------------------- */
    #define RX_COMPILER "clang" __clang_version__
#elif defined(__GNUG__) && !(defined(__clang__) || defined(__INTEL_COMPILER))
    /* GNU GCC/G++. --------------------------------------------- */
    #define RX_COMPILER "g++" __VERSION__
#elif defined(_MSC_VER)
    /* Microsoft Visual Studio. --------------------------------- */
    #include <string>
    #define RX_COMPILER "msvc" + std::to_string(_MSC_FULL_VER)
#else
    #error Unsupported compiler
#endif

// Combined build info
#define RX_BUILD (RX_OS "-" RX_COMPILER)

#endif
