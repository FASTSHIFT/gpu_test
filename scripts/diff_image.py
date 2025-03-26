"""
MIT License

Copyright (c) 2025 _VIFEXTech

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

from PIL import Image, ImageChops
import argparse


def main(image1_path, image2_path, output_path, highlight_color):
    try:
        # Open both images
        im1 = Image.open(image1_path)
        im2 = Image.open(image2_path)
    except IOError as e:
        print(f"Failed to open image: {e}")
        return 1  # Return 1 to indicate an error

    # Compare the two images
    diff = ImageChops.difference(im1, im2)

    # Generate a highlighted image of inconsistent pixels
    if diff.getbbox():  # If there are different parts
        # Create an overlay layer with the specified highlight color
        highlight_color_rgba = (*highlight_color, 128)
        highlight_overlay = Image.new("RGBA", im1.size, highlight_color_rgba)

        # Create a mask with 255 only on the difference part
        diff_mask = diff.convert("L")  # Convert the difference image to grayscale
        diff_mask = diff_mask.point(lambda p: p > 0 and 255)  # Create a binary mask

        # Apply the highlight overlay layer to the original image
        combined = Image.composite(highlight_overlay, im1.convert("RGBA"), diff_mask)

        try:
            combined.save(output_path)
            print(f"Saved difference image: {output_path}")
            return 1  # Return 1 to indicate a difference was found and saved
        except IOError as e:
            print(f"Failed to save image: {e}")
            return 1  # Return 1 to indicate an error
    else:
        print(f"No difference: {image1_path}")
        return 0  # Return 0 to indicate no difference was found


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Compare two images and highlight differences."
    )
    parser.add_argument(
        "-i", "--image1-path", type=str, required=True, help="Path to the first image"
    )
    parser.add_argument(
        "-d", "--image2-path", type=str, required=True, help="Path to the second image"
    )
    parser.add_argument(
        "-o",
        "--output-path",
        type=str,
        required=True,  # Made the output path required for clarity
        help="Path to save the output difference image",
    )
    parser.add_argument(
        "-c",
        "--highlight-color",
        type=str,
        default="(255, 0, 0)",
        help="Highlight color in RGB format (default: (255, 0, 0))",
    )

    args = parser.parse_args()

    # Convert the highlight color string to a tuple
    try:
        highlight_color = tuple(map(int, args.highlight_color.strip("()").split(",")))
        if len(highlight_color) != 3:
            raise ValueError("Highlight color must be in RGB format with 3 values.")
    except ValueError as e:
        print(f"Invalid highlight color format: {e}")
        exit(1)

    # Call the main function with parsed arguments and handle the return value
    result = main(args.image1_path, args.image2_path, args.output_path, highlight_color)
    exit(result)
