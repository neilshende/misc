import pandas as pd
import numpy as np
import matplotlib.pyplot as plt 
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LinearRegression
from sklearn.linear_model import Ridge, RidgeCV, Lasso
from sklearn.preprocessing import StandardScaler
#from sklearn.datasets import load_boston
#from sklearn.datasets import fetch_openml

data_url = "http://lib.stat.cmu.edu/datasets/boston"
raw_df = pd.read_csv(data_url, sep="\s+", skiprows=22, header=None)
data = np.hstack([raw_df.values[::2, :], raw_df.values[1::2, :2]])
target = raw_df.values[1::2, 2]
X_train, X_test, y_train, y_test = train_test_split(data, target, test_size=0.3, random_state=17)

scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

ridgeReg = Ridge(alpha=10)
ridgeReg.fit(X_train, y_train)
y_pred = ridgeReg.predict(X_test)
plt.scatter(y_pred, y_test)
plt.show()
lassoReg = Lasso(alpha=.1)
lassoReg.fit(X_train, y_train)
y2_pred = lassoReg.predict(X_test)
plt.scatter(y2_pred, y_test)
plt.show()
