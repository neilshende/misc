       01 SQLCA.
          05 SQLCAID     PIC X(8).
          05 SQLCABC     PIC S9(9) COMP.
          05 SQLCODE     PIC S9(9) COMP.
          05 SQLERRM.
             10 SQLERRML PIC S9(4) COMP.
             10 SQLERRMC PIC X(70).
          05 SQLERRP     PIC X(8).
          05 SQLERRD     PIC S9(9) COMP OCCURS 6 TIMES.
          05 SQLWARN.
             10 SQLWARN0 PIC X.
             10 SQLWARN1 PIC X.
          05 SQLEXT      PIC X(8).
