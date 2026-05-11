       IDENTIFICATION DIVISION.
       PROGRAM-ID. CALLEE.

       DATA DIVISION.

       WORKING-STORAGE SECTION.
       01  WS-RESULT      PIC 9(5).

       LINKAGE SECTION.
       01  LK-NUM1        PIC 9(3).
       01  LK-NUM2        PIC 9(3).
       01  LK-SUM         PIC 9(5).

       PROCEDURE DIVISION USING LK-NUM1 LK-NUM2 LK-SUM.

           ADD LK-NUM1 LK-NUM2
               GIVING WS-RESULT

           MOVE WS-RESULT TO LK-SUM

           GOBACK.

       IDENTIFICATION DIVISION.
       PROGRAM-ID. CALLER.

       DATA DIVISION.

       WORKING-STORAGE SECTION.
       01  WS-A           PIC 9(3) VALUE 100.
       01  WS-B           PIC 9(3) VALUE 250.
       01  WS-SUM         PIC 9(5).

       PROCEDURE DIVISION.

           CALL "CALLEE"
               USING WS-A
                     WS-B
                     WS-SUM

           DISPLAY "SUM = " WS-SUM

           STOP RUN.

