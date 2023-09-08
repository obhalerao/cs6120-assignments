# ---
# jupyter:
#   jupytext:
#     formats: ipynb,py:light
#     text_representation:
#       extension: .py
#       format_name: light
#       format_version: '1.5'
#       jupytext_version: 1.15.1
#   kernelspec:
#     display_name: Python 3 (ipykernel)
#     language: python
#     name: python3
# ---

import pandas as pd
import numpy as np
from pathlib import Path
from bokeh.io import output_notebook # enables plot interface in J notebook
_ = output_notebook()
from bokeh.models import ColumnDataSource
from bokeh.plotting import figure, show
from bokeh.transform import dodge
from bokeh.palettes import Accent7 as palette

results_raw = pd.read_csv('results.csv')
results = results_raw.pivot(index='benchmark', columns='run', values='result').replace({'missing': 'NaN', 'timeout': 'NaN'}).astype("float").reset_index()

results['benchmark_name'] = results['benchmark'].apply(lambda s: (Path("..") / s).stem)
results['benchmark_group'] = results['benchmark'].apply(lambda s: (Path("..") / s).parts[-2])
speedups = []
for opt in ['baseline', 'tdce', 'lvn']:
    speedups.append(speedup := f'{opt}_speedup')
    results[speedup] = results['baseline'] / results[opt]
results

for name, group in results.groupby('benchmark_group'):
    data = group
    benches = list(data['benchmark_name'])
    speedups = list(col for col in data.columns if 'speedup' in col)
    
    source = ColumnDataSource(data=data)
    
    p = figure(x_range=benches, y_range=(0, 2.5), title=f"Speedups for {name} Benchmarks",
               height=350, width=1050, toolbar_location=None, tools="")
    n = len(speedups)
    for i, speedup in enumerate(speedups):
        _ = p.vbar(x=dodge('benchmark_name', 1 / (n + 1) * (i - 1), range=p.x_range), top=speedup, source=source, width=1 / (n + 2), color=palette[i], legend_label=speedup)
    p.x_range.range_padding = 0.1
    p.xgrid.grid_line_color = None
    p.xaxis.major_label_orientation = np.pi/4
    p.legend.location = "top_left"
    p.legend.orientation = "horizontal"
    show(p)

avg_speedups = results.groupby('benchmark_group')[speedups].mean()

avg_speedups

print("all done!")
