       IDENTIFICATION DIVISION.
       PROGRAM-ID. READPRINT.

       ENVIRONMENT DIVISION.
       INPUT-OUTPUT SECTION.
       FILE-CONTROL.
           SELECT INFILE
               ASSIGN TO "input.txt"
               ORGANIZATION IS LINE SEQUENTIAL
               FILE STATUS IS WS-FILE-STATUS.

       DATA DIVISION.
       FILE SECTION.
       FD  INFILE.
       01  IN-REC            PIC X(132).

       WORKING-STORAGE SECTION.
       01  WS-FILE-STATUS    PIC XX.
       01  WS-EOF            PIC X VALUE "N".

       PROCEDURE DIVISION.
       MAIN-PARA.

           OPEN INPUT INFILE
           IF WS-FILE-STATUS NOT = "00"
               DISPLAY "ERROR OPENING FILE, STATUS=" WS-FILE-STATUS
               STOP RUN
           END-IF

           PERFORM UNTIL WS-EOF = "Y"
               READ INFILE
                   AT END
                       MOVE "Y" TO WS-EOF
                   NOT AT END
                       DISPLAY IN-REC
               END-READ
           END-PERFORM

           CLOSE INFILE

           STOP RUN.

