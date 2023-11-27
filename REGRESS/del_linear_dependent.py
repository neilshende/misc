import pandas as pd
import sympy
columns = ['A','B', 'C', 'D']
df = pd.DataFrame(columns=columns)
df.A = [0,2,3,4]
df.C = [8,3,5,4]
df.B = df.A*2 + df.C*3
df.D = df.C*3
reduced_form, inds = sympy.Matrix(df.values).rref()
#Next line is not working
#df.iloc[:, inds]
#Hack out our own implementation to slice the table
for x in range(len(columns)):
   if x in inds:
      pass
   else:
      print(columns[x])
      df = df.drop(columns[x], axis=1)
