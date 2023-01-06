#include "Log.h"

ArrayList<Log::Printer> Log::s_Printers;
String Log::s_Severities[] = {"[E]", "[W]", "[I]", "[D]"};

void Log::attach(Print& printer, const Level& level) {
    detach(printer);
    s_Printers.add(Printer(&printer, level));
}

void Log::detach(Print& printer) {
    for (int i = 0; i < s_Printers.size(); i++) {
        if (s_Printers[i].printer == &printer) {
            s_Printers.removeAt(i);
            break;
        }
    }
}
