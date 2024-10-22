#include "main.hpp"



void testCustomer() {
    Customer c;
    c.name = "Mario";
    c.surname = "Rossi";
    c.ID = 1;

    printf("Customer: %s %s, ID: %d\n", c.name.c_str(), c.surname.c_str(), c.ID);
}

int main() {
    printf("Ciao!\n");
    testCustomer();
}

