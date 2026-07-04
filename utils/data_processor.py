import pandas as pd
from typing import List, Dict, Any

def load_data(file_path: str, file_type: str = 'csv') -> pd.DataFrame:
    """
    Loads data from various supported files (CSV, Excel, etc.) into a pandas DataFrame.

    Args:
        file_path: The absolute path to the data file.
        file_type: The type of file ('csv', 'excel'). Defaults to 'csv'.

    Returns:
        A pd.DataFrame containing the loaded data.

    Raises:
        ValueError: If the file_type is not supported.
    """
    if file_type == 'csv':
        try:
            df = pd.read_csv(file_path)
            return df
        except FileNotFoundError:
            raise FileNotFoundError(f"The file was not found at: {file_path}")
    elif file_type == 'excel':
        try:
            # Assuming the first sheet if multiple exist
            df = pd.read_excel(file_path, engine='openpyxl')
            return df
        except FileNotFoundError:
            raise FileNotFoundError(f"The file was not found at: {file_path}")
    else:
        raise ValueError(f"Unsupported file type: {file_type}. Please use 'csv' or 'excel'.")

def clean_column(df: pd.DataFrame, column_name: str, cleaning_method: str = 'strip') -> pd.DataFrame:
    """
    Cleans a specified column in the DataFrame.

    Args:
        df: The input pandas DataFrame.
        column_name: The name of the column to clean.
        cleaning_method: The method to apply ('strip' for whitespace, 'lowercase' for case).

    Returns:
        A new DataFrame with the cleaned column.
    """
    if column_name not in df.columns:
        raise KeyError(f"Column '{column_name}' not found in the DataFrame.")

    df_cleaned = df.copy()

    if cleaning_method == 'strip':
        # Assumes string type for stripping whitespace
        df_cleaned[column_name] = df_cleaned[column_name].astype(str).str.strip()
    elif cleaning_method == 'lowercase':
        # Converts all strings to lowercase
        df_cleaned[column_name] = df_cleaned[column_name].astype(str).str.lower()
    else:
        raise ValueError("Unsupported cleaning method. Use 'strip' or 'lowercase'.")

    return df_cleaned

def calculate_summary_stats(series: pd.Series) -> Dict[str, Any]:
    """
    Calculates basic summary statistics for a given pandas Series.

    Args:
        series: The pandas Series to analyze.

    Returns:
        A dictionary containing mean, median, min, max, and count.
    """
    if pd.api.types.is_numeric_dtype(series):
        return {
            "count": series.count(),
            "mean": series.mean(),
            "median": series.median(),
            "min": series.min(),
            "max": series.max()
        }
    else:
        return {"error": "Series must be numeric to calculate standard summary statistics."}

def transform_data(df: pd.DataFrame, column_to_transform: str, transformation_func) -> pd.DataFrame:
    """
    Applies a custom function (transformation_func) to a specified column.

    Args:
        df: The input pandas DataFrame.
        column_to_transform: The name of the column to apply the function to.
        transformation_func: A callable function that takes one argument (a value from the column) and returns a transformed value.

    Returns:
        A new DataFrame with the transformed column.
    """
    if column_to_transform not in df.columns:
        raise KeyError(f"Column '{column_to_transform}' not found for transformation.")

    df_transformed = df.copy()
    # Apply the function to every element in the specified column
    df_transformed[f'{column_to_transform}_transformed'] = df_transformed[column_to_transform].apply(transformation_func)

    return df_transformed