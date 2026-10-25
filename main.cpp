#include "PortController.h"
#include "PortMenu.h"

int main() {
    PortController controller;
    PortMenu menu(controller);
    menu.run();
    return 0;
}
