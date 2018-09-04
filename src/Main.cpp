# include <iostream>
# include <chrono>
# include <list>
# include <getopt.h>

# include "RadixTree.h"

# define        HELP "Usage : %s -d -h\n" \
                     "\n" \
                     "\t-d:             Enable the debug CLI.\n" \
                     "\t-h:\t\tDisplay this screen.\n"

bool g_debug = false;

bool  checkArgs(int ac, char **av)
{
  int c;

  while ((c = getopt (ac, av, "dh")) != -1)
    switch (c) {
      case 'h':
        fprintf (stderr, HELP, av[0]);
        return false;
      case 'd':
        g_debug = true;
        break;
      case '?':
        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        fprintf (stderr, HELP, av[0]);
        return false;
      default:
        fprintf (stderr, HELP, av[0]);
        return false;
    }

  return true;
}

void feedTree(std::shared_ptr<RadixTree> & radixTree)
{
  std::string                line;
  int                        val = 0;
  bool                       running = true;
  std::list<std::string>      garbage;

  if (g_debug) {
    do {
      std::cout << "Enter some key to add (or start to start to search) : ";
      std::getline(std::cin, line);
      if (line == "start")
        running = false;
      else {
        garbage.push_back(std::to_string(val));
        radixTree->loadString(line, garbage.end()->c_str());
      }
      val++;
    } while (running);
  } else {
    radixTree->loadString("deck", &val);
    radixTree->loadString("decker", &val);
    radixTree->loadString("did", &val);
    radixTree->loadString("doe", &val);
    radixTree->loadString("dog", &val);
    radixTree->loadString("dogs", &val);
    radixTree->loadString("doge", &val);
    radixTree->loadString("start", &val);
  }
}

void searchTree(std::shared_ptr<RadixTree> & radixTree)
{
  std::string                line;
  bool                       running = true;

    if (g_debug) {
      do {
        std::cout << "Enter a key to search (or quit/exit to exit) : ";
        std::getline(std::cin, line);
        if (line == "quit" || line == "exit")
          running = false;
        else {

          std::cout << "\n-----------------------\nResearch of :" << line << std::endl;
          auto t1_lpm = std::chrono::system_clock::now();
          auto lpm = radixTree->longestPrefixMatch(line);
          auto t2_lpm = std::chrono::system_clock::now();
          std::chrono::duration<double> diff_lpm = t2_lpm - t1_lpm;
          std::cout << "\nLongest Prefix Match : \n" << *lpm << ": " << diff_lpm.count() << "s." << std::endl;

          auto t1_pm = std::chrono::system_clock::now();
          auto pm = radixTree->perfectMatch(line);
          auto t2_pm = std::chrono::system_clock::now();
          std::chrono::duration<double> diff_pm = t2_pm - t1_pm;
          std::cout << "\nPerfect Match : \n" << pm.first << ": [" << pm.second << "]: " << diff_pm.count() << "s." << std::endl;
        }
        line.clear();
      } while (running);
    }
}

int main(int ac, char ** av)
{
  if (!checkArgs(ac, av))
    return -1;

  std::shared_ptr<RadixTree> radixTree = std::make_unique<RadixTree>();

  feedTree(radixTree);
  radixTree->print();
  searchTree(radixTree);

  return 0;
}
