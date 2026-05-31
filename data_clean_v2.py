import pygeohash as pgh
import pandas as pd

def load_data(data_file_path):
  df = pd.read_csv(data_file_path)
  return df

def remove_na(df : pd.DataFrame):
  print(df.isnull().sum())
  df_cleaned = df.dropna()
  return df_cleaned

df_encoded = pd.get_dummies(df_cleaned, columns=['Weather'], prefix='Weather')
df_encoded.dropna(subset=['Temperature'], inplace=True)

df_encoded

df_encoded = pd.get_dummies(df_encoded, columns=['RoadType'], prefix='Road')

df_encoded.head()

df_encoded['LargeVehicles'] = df_encoded['LargeVehicles'].map({'Allowed' : 1, 'Not Allowed' : 0})
df_encoded['Landmarks'] = df_encoded['Landmarks'].map({'Yes' : 1, 'No' : 0})

df_encoded

df_encoded['latitude'] = None
df_encoded['longitude'] = None

def decode_geohash(geohash_string):
  if pd.isna(geohash_string) or geohash_string == '':
    return None, None
  else:
    return pgh.decode(geohash_string)


df_encoded[['latitude', 'longitude']] = df_encoded['geohash'].apply(lambda x : pd.Series(decode_geohash(x)))

df_encoded

df_encoded = df_encoded.drop(columns = ['geohash'])

df_encoded.head()

df_encoded.to_csv('/content/gridlock2_dataset/dataset/cleaned_data.csv', index=False)


