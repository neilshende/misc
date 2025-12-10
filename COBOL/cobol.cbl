       ************
       *CREATE TABLE CUSTOMER (
       *    CUST_ID     INTEGER     NOT NULL PRIMARY KEY,
       *    NAME        VARCHAR(50),
       *    EMAIL       VARCHAR(50)
       *);
       ************
       IDENTIFICATION DIVISION.
       PROGRAM-ID. CUSTOMERCRUD.

       ENVIRONMENT DIVISION.

       DATA DIVISION.
       WORKING-STORAGE SECTION.

       01  WS-SQLCODE        PIC S9(9) COMP.
       01  WS-CUST-ID        PIC 9(9).
       01  WS-NAME           PIC X(50).
       01  WS-EMAIL          PIC X(50).

       01  MENU-OPTION       PIC 9.
       01  DONE              PIC X VALUE "N".

       EXEC SQL INCLUDE SQLCA END-EXEC.

       PROCEDURE DIVISION.
       MAIN-PARA.
           PERFORM UNTIL DONE = "Y"
              DISPLAY " "
              DISPLAY "==== CUSTOMER CRUD MENU ===="
              DISPLAY "1. CREATE CUSTOMER"
              DISPLAY "2. READ CUSTOMER"
              DISPLAY "3. UPDATE CUSTOMER"
              DISPLAY "4. DELETE CUSTOMER"
              DISPLAY "5. EXIT"
              DISPLAY "Choose: "
              ACCEPT MENU-OPTION

              EVALUATE MENU-OPTION
                 WHEN 1  PERFORM CREATE-CUSTOMER
                 WHEN 2  PERFORM READ-CUSTOMER
                 WHEN 3  PERFORM UPDATE-CUSTOMER
                 WHEN 4  PERFORM DELETE-CUSTOMER
                 WHEN 5  MOVE "Y" TO DONE
                 WHEN OTHER
                    DISPLAY "Invalid option."
              END-EVALUATE
           END-PERFORM.

           STOP RUN.

       *****************************************************************
       * CREATE: Insert a new customer
       *****************************************************************
       CREATE-CUSTOMER.
           DISPLAY "Enter Customer ID: " ACCEPT WS-CUST-ID.
           DISPLAY "Enter Name:        " ACCEPT WS-NAME.
           DISPLAY "Enter Email:       " ACCEPT WS-EMAIL.

           EXEC SQL
                INSERT INTO CUSTOMER (CUST_ID, NAME, EMAIL)
                VALUES (:WS-CUST-ID, :WS-NAME, :WS-EMAIL)
           END-EXEC.

           MOVE SQLCODE TO WS-SQLCODE.
           IF WS-SQLCODE = 0
               DISPLAY "Customer created successfully."
           ELSE
               DISPLAY "Error inserting. SQLCODE=" WS-SQLCODE.

       *****************************************************************
       * READ: Retrieve by customer ID
       *****************************************************************
       READ-CUSTOMER.
           DISPLAY "Enter Customer ID to read: "
           ACCEPT WS-CUST-ID.

           EXEC SQL
                SELECT NAME, EMAIL
                  INTO :WS-NAME, :WS-EMAIL
                  FROM CUSTOMER
                 WHERE CUST_ID = :WS-CUST-ID
           END-EXEC.

           MOVE SQLCODE TO WS-SQLCODE.

           IF WS-SQLCODE = 0
              DISPLAY "Customer Found:"
              DISPLAY "  Name : " WS-NAME
              DISPLAY "  Email: " WS-EMAIL
           ELSE
              DISPLAY "Customer not found. SQLCODE=" WS-SQLCODE.

       *****************************************************************
       * UPDATE: Update name or email
       *****************************************************************
       UPDATE-CUSTOMER.
           DISPLAY "Enter Customer ID to update: "
           ACCEPT WS-CUST-ID.

           DISPLAY "Enter New Name:  " ACCEPT WS-NAME.
           DISPLAY "Enter New Email: " ACCEPT WS-EMAIL.

           EXEC SQL
               UPDATE CUSTOMER
                  SET NAME  = :WS-NAME,
                      EMAIL = :WS-EMAIL
                WHERE CUST_ID = :WS-CUST-ID
           END-EXEC.

           MOVE SQLCODE TO WS-SQLCODE.

           IF WS-SQLCODE = 0
              DISPLAY "Customer updated successfully."
           ELSE
              DISPLAY "Update failed. SQLCODE=" WS-SQLCODE.

       *****************************************************************
       * DELETE: Delete by ID
       *****************************************************************
       DELETE-CUSTOMER.
           DISPLAY "Enter Customer ID to delete: "
           ACCEPT WS-CUST-ID.

           EXEC SQL
              DELETE FROM CUSTOMER
               WHERE CUST_ID = :WS-CUST-ID
           END-EXEC.

           MOVE SQLCODE TO WS-SQLCODE.

           IF WS-SQLCODE = 0
              DISPLAY "Customer deleted successfully."
           ELSE
              DISPLAY "Delete failed. SQLCODE=" WS-SQLCODE.

