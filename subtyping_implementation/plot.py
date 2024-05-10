import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

sns.set_theme(style="darkgrid")

sns.set_context("talk", font_scale=1.8)

worst_case_results = pd.read_csv('results_worst_case.txt')
iso_results = pd.read_csv('results_isomorphic.txt')
idemp_results = pd.read_csv('results_idempotent.txt')
unfolded_results = pd.read_csv('results_unfolded.txt')

def plot(df, title, iters=1):
    algos = df['algorithm'].unique()
    df["time_seconds"] = df['time'] / 1e9 / iters
    for algo in algos:
        df_algo = df[df['algorithm'] == algo]
        successful = df_algo[df_algo['success'] == 1]
        sns.lineplot(successful, x="k", y="time_seconds", label=f"{algo} successes")
        unsuccessful = df_algo[df_algo['success'] == 0]
        sns.scatterplot(unsuccessful, x="k", y="time_seconds", label=f"{algo} timeouts", marker='x')
    plt.title(title)
    plt.xlabel('k')
    plt.ylabel('time (s)')
    plt.yscale('log')

def plot_scatter(df, title, iters=1):
    df["time_seconds"] = df['time'] / 1e9 / iters
    df = df.pivot(index="k", columns="algorithm", values="time_seconds")
    sns.scatterplot(data=df, x="inductive", y="coinductive")
    plt.title(title)
    plt.xlabel('inductive')
    plt.ylabel('coinductive')
    # plt.xscale('log')
    # plt.yscale('log')

plot(worst_case_results, "Runtime of exponential test case")
plt.tight_layout(pad=1.2)
plt.savefig("worst_case.svg")
plt.clf()
plot(iso_results, "Runtime of random unfolded test cases")
plt.savefig("iso.svg")
plt.clf()
plot_scatter(idemp_results, "Runtime of random idempotent test cases", iters=10000)
plt.savefig("idemp.svg")
plt.clf()
plot_scatter(unfolded_results, "Runtime of random unfolded test cases", iters=10000)
plt.savefig("unfolded.svg")
plt.clf()

