       IDENTIFICATION DIVISION.
       PROGRAM-ID. GENERATE-EQUATION.

       DATA DIVISION.
       WORKING-STORAGE SECTION.
       01  WS-OFFSET        PIC 9(9) COMP VALUE 0.
       01  WS-SUM           PIC S9(9) COMP VALUE 0.
       01  WS-FIRST-FLAG    PIC X VALUE 'Y'.
       01  WS-I             PIC S9(9) COMP.
       01  WS-NUM-TEXT      PIC X(20).
       01  WS-LEN           PIC 9(9) COMP.

       LINKAGE SECTION.
       01  LK-A             PIC S9(9) COMP.
       01  LK-B             PIC S9(9) COMP.
       01  LK-BUF.
           05 LK-BUF-DATA   PIC X OCCURS 0 TO 9999
                               DEPENDING ON LK-BUFLEN.
       01  LK-BUFLEN        PIC S9(9) COMP.
       01  LK-RET           PIC S9(9) COMP.

       PROCEDURE DIVISION
           USING LK-A LK-B LK-BUF LK-BUFLEN LK-RET.

       MAIN-LOGIC.
           MOVE 0 TO WS-OFFSET WS-SUM
           MOVE 'Y' TO WS-FIRST-FLAG

           IF LK-BUFLEN <= 0
              MOVE -3 TO LK-RET
              GOBACK
           END-IF

           IF LK-A > LK-B
              MOVE -4 TO LK-RET
              GOBACK
           END-IF

           PERFORM VARYING WS-I FROM LK-A BY 1
                   UNTIL WS-I > LK-B

              IF FUNCTION MOD(WS-I 3) = 0
                 OR FUNCTION MOD(WS-I 5) = 0

                 MOVE SPACES TO WS-NUM-TEXT

                 IF WS-FIRST-FLAG = 'Y'
                    MOVE WS-I TO WS-NUM-TEXT
                    MOVE 'N' TO WS-FIRST-FLAG
                 ELSE
                    STRING "+"
                           FUNCTION TRIM(WS-I)
                           INTO WS-NUM-TEXT
                    END-STRING
                 END-IF

                 COMPUTE WS-LEN =
                         LENGTH(FUNCTION TRIM(WS-NUM-TEXT))

                 IF WS-OFFSET + WS-LEN > LK-BUFLEN
                    MOVE -1 TO LK-RET
                    GOBACK
                 END-IF

                 STRING FUNCTION TRIM(WS-NUM-TEXT)
                        INTO LK-BUF
                        WITH POINTER WS-OFFSET
                 END-STRING

                 ADD WS-I TO WS-SUM
              END-IF
           END-PERFORM

           IF WS-SUM > 0
              MOVE SPACES TO WS-NUM-TEXT
              STRING "="
                     FUNCTION TRIM(WS-SUM)
                     INTO WS-NUM-TEXT
              END-STRING

              COMPUTE WS-LEN =
                      LENGTH(FUNCTION TRIM(WS-NUM-TEXT))

              IF WS-OFFSET + WS-LEN > LK-BUFLEN
                 MOVE -1 TO LK-RET
                 GOBACK
              END-IF

              STRING FUNCTION TRIM(WS-NUM-TEXT)
                     INTO LK-BUF
                     WITH POINTER WS-OFFSET
              END-STRING
           END-IF

           MOVE 0 TO LK-RET
           GOBACK.

