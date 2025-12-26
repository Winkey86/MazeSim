#include "ui/App.h"
#include <iostream>

int main() {
    try {
        ml::ui::App app;
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
