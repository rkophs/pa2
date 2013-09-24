//
//  main.c
//  test
//
//  Created by Ryan Kophs on 9/23/13.
//  Copyright (c) 2013 edu.kophs. All rights reserved.
//

#include <stdio.h>
#include "window_.h"

int main(int argc, const char * argv[])
{

    struct window *test = windowInit(6, 8);
    char * a = "000";
    char * b = "001";
    char * c = "002";
    char * d = "003";
    char * e = "004";
    char * f = "005";
    char * g = "006";
    char * h = "007";
    char * i = "008";
    char * j = "009";
    char * k = "10";
    char * l = "11";
    
    insertSubBuffer(test, 0, a, 3);
    printWindow(test);
    insertSubBuffer(test, 1, b, 3);
    printWindow(test);
    insertSubBuffer(test, 2, c, 3);
    printWindow(test);
    insertSubBuffer(test, 3, d, 3);
    printWindow(test);
    insertSubBuffer(test, 4, e, 3);
    printWindow(test);
    insertSubBuffer(test, 5, f, 3);
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    insertSubBuffer(test, 6, g, 3);
    printWindow(test);
    insertSubBuffer(test, 1, d, 3);
    printWindow(test);
    insertSubBuffer(test, 2, d, 3);
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n\n", test->cumSeq, test->min);
    
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    
    printf("REDO\n");
    insertSubBuffer(test, 0, a, 3);
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    insertSubBuffer(test, 1, b, 3);
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    insertSubBuffer(test, 2, c, 3);
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    insertSubBuffer(test, 7, b, 3);
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    insertSubBuffer(test, 9, c, 3);
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    insertSubBuffer(test, 6, b, 3);
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    insertSubBuffer(test, 8, c, 3);
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    
    printf("NOW SHIFTING WINDOW\n");
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    printf("Shift: %s\n", shiftWindow(test));
    printWindow(test);
    printf("CUM ACK AND MIN: %i, %i\n", test->cumSeq, test->min);
    
    freeWindow(test);

    return 0;
}

