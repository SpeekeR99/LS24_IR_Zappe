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
    :param document_path: Path to document
    """
    # Load model
    model = load_model(model_path)

    # Load document
    with open(document_path, "r", encoding="utf-8", errors="ignore") as fp:
        document = fp.read()

    # Predict language
    y_pred = model.predict([document])

    # Load label2lang mapping
    label2lang = load_label2lang()

    # Convert label to language
    label = label2lang[y_pred[0]]

    print(f"Predicted language: {label}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <path_to_model> <path_to_document>")
        sys.exit(1)

    main(sys.argv[1], sys.argv[2])
