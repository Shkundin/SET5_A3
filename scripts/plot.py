import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

df=pd.read_csv("data/stats.csv")
x=df["processed"].to_numpy(dtype=float)
y_true=df["mean_exact"].to_numpy(dtype=float)
y_est=df["mean_est"].to_numpy(dtype=float)
sigma=df["std_est"].to_numpy(dtype=float)

plt.figure()
plt.plot(x,y_true,label="F_t0")
plt.plot(x,y_est,label="N_t")
plt.xlabel("Processed elements")
plt.ylabel("Unique elements")
plt.legend()
plt.tight_layout()
plt.savefig("report/graph1.png",dpi=200)

plt.figure()
plt.plot(x,y_est,label="E(N_t)")
plt.fill_between(x,y_est-sigma,y_est+sigma,alpha=0.3,label="±σ")
plt.xlabel("Processed elements")
plt.ylabel("Estimated unique elements")
plt.legend()
plt.tight_layout()
plt.savefig("report/graph2.png",dpi=200)
