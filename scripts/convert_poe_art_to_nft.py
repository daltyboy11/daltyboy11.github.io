import os
import json
import shutil

# Define paths
json_path = "./scripts/poe_nft_art_map.json"
input_images_dir = "./images/edgar-allan-poe-art/"
output_images_dir = "./images/edgar-allan-poe-nft/"

# Load JSON data
with open(json_path, 'r') as json_file:
    art_map = json.load(json_file)

# Validation: Check if all image paths exist
missing_files = []
for key, value in art_map.items():
    print(key)
    print(value)
    image_path = os.path.join(input_images_dir, value['image'])
    if not os.path.exists(image_path):
        missing_files.append(image_path)

# If any image files are missing, raise an error and list them
if missing_files:
    missing_files_str = "\n".join(missing_files)
    raise FileNotFoundError(f"The following image paths do not exist:\n{missing_files_str}")

# Proceed to file copying if validation passes
os.makedirs(output_images_dir, exist_ok=True)

for key, value in art_map.items():
    image_path = os.path.join(input_images_dir, value['image'])
    output_path = os.path.join(output_images_dir, f"{key}")
    
    # Copy the image file to the new location with the new name
    shutil.copy(image_path, output_path)
    print(f"Copied {image_path} to {output_path}")

print("All images processed successfully.")
