import pandas as pd
from sklearn.preprocessing import MinMaxScaler
import joblib
import numpy as np
import json

df = pd.read_csv("training_data.csv")

#Since sensors send data at a high freq and in this case 1 bin_ id = 1sec we need to 
# group them so that every pgn appears once with a total count and mean for iat/jitter
df_consolidate = df.groupby(['bin_id', 'pgn']).agg({
    'count':'sum',
    'iat_count':'sum',
    'jitter_count':'sum',
    'sum_iat': 'sum',
    'max_iat': 'max',
    'sum_jitter':'sum',
}).reset_index()

#Calc the average IAT and Jitter based on the respective counts without skew
df_consolidate['avg_iat'] = df_consolidate['sum_iat'] / df_consolidate['iat_count']
df_consolidate['avg_jitter'] = df_consolidate['sum_jitter'] / df_consolidate['jitter_count']


#Drop the unneccesary columns and replace all 0 divisions / inf numbers with a 0.
df_consolidate = df_consolidate.drop(columns=['sum_iat','sum_jitter','iat_count','jitter_count'])
df_consolidate.replace([np.inf, -np.inf], np.nan, inplace=True)
df_consolidate.fillna(0,inplace=True)


#Pivot the table and use bin_id to align the rows (this is not given to the model as it would freak out if bin_id goes above whats seen during training)
pivoted = df_consolidate.pivot(index='bin_id', columns='pgn', values=['count', 'avg_iat', 'max_iat', 'avg_jitter'])

#If the sensor is silent add a 0 in its place.
# NOTE_TO_SELF: Some sensors send maybe 100 packets out of 1 million, these could be removed as theyre not chatty enough
pivoted.fillna(0, inplace=True)

#col[0] = metric
#col[1] = PGN number
pivoted.columns = [f'{col[0]}_{int(col[1])}' for col in pivoted.columns] #Flatten

feature_list = list(pivoted.columns)

#Save the feature list
with open('feature_table.json', 'w') as f:
    json.dump(feature_list, f)

scaler = MinMaxScaler()
scaled_data = scaler.fit_transform(pivoted)

np.save('scaled_training_data.npy', scaled_data)

joblib.dump(scaler, 'min_max_scaler.pkl')



