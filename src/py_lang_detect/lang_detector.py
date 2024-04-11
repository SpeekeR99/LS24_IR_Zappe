import os
import sys
import pickle
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.naive_bayes import MultinomialNB
from sklearn.pipeline import Pipeline


def load_model(path):
    """
    Load model from a given path
    :param path: Path where model is saved
    :return: Model
    """
    with open(path, "rb") as f:
        return pickle.load(f)


def load_label2lang():
    """
    Create mapping of labels to languages
    :return: Mapping of labels to languages
    """
    # Mapping of labels to languages
    label2lang = {}
    languages = ["cs", "de", "en", "es", "fr", "it", "pl", "pt", "ru", "sk"]

    for i, lang in enumerate(languages):
        label2lang[i] = lang

    return label2lang


def main(model_path, document_path):
    """
    Main function
    :param model_path: Path to model
    :param document_path: Path to document dir
    """
    # Load model
    model = load_model(model_path)

    # Load document
    document = []
    ids = []
    for file in os.listdir(document_path):
        filepath = os.path.join(document_path, file)
        with open(filepath, "r", encoding="utf-8") as fp:
            document.append(fp.read())
            file_without_ext = os.path.splitext(file)[0]
            ids.append(file_without_ext)

    # Predict language
    y_pred = model.predict(document)

    # Load label2lang mapping
    label2lang = load_label2lang()

    # Convert label to language
    labels = [label2lang[label] for label in y_pred]

    result = dict(zip(ids, labels))
    print(f"Predicted languages: {result}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <path_to_model> <path_to_document_dir>")
        sys.exit(1)

    print(sys.argv)

    main(sys.argv[1], sys.argv[2])
