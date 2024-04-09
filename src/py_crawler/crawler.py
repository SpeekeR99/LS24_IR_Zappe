import sys
import json
from selenium import webdriver
from selenium.webdriver.common.by import By

# Filter for the seed pages where to find links to other SEED pages
XPATH_NEXT_PAGE = "//div[@class='mw-allpages-nav']//a[@href]"
# Filter for the seed pages where to find links to CONTENT pages
XPATH_LINKS = "//div[@class='mw-allpages-body']//a[@href]"
# Filters for the content of the pages
XPATH_CONTENT = {
    "title": "//span[@class='mw-page-title-main']",
    "toc": "//div[@id='content']//div[@id='toc']",
    "h1": "//div[@id='content']//h1",
    "h2": "//div[@id='content']//h2",
    "h3": "//div[@id='content']//h3",
    "content": "//div[@id='content']"
}


class WitcherCrawler:
    """
    Very simple web crawler using Selenium
    Basically only works for my website (witcher fandom wiki)
    """
    def __init__(self):
        """
        Initialize the web driver
        """
        self.driver = webdriver.Firefox()

    def quit(self):
        """
        Quit the web driver
        """
        self.driver.quit()

    def process_url(self, url, xpath_map):
        """
        Process a single URL with the given xpath map
        :param url: URL to process
        :param xpath_map: Map of XPath filters to apply
        :return: Results of the processing
        """
        results = {}

        print("Processing: " + url)

        self.driver.get(url)
        dom = self.driver.page_source

        if dom is not None:  # If the page was fetched successfully
            for key in xpath_map.keys():
                try:  # Try to find the element with the given xpath
                    result_list = self.driver.find_elements(By.XPATH, xpath_map[key])
                    results[key] = []

                    for result in result_list:  # Add the inner text of the element to the results
                        results[key].append(result.get_attribute("innerText"))

                except Exception as e:  # If the element was not found
                    print("Couldn't find the element with xpath: " + xpath_map[key])
                    print(str(e))
        else:  # If the page was not fetched successfully
            print("Couldn't fetch the content of the page.")

        return results


def download_webpage_as_json(crawler, url, xpath_filter):
    """
    Download the content of the given URL and save it as a JSON file
    :param crawler: Crawler to use
    :param url: URL to download
    :param xpath_filter: Filter for the content of the page
    """
    data = crawler.process_url(url, xpath_filter)

    try:  # Try to write the data to a file
        title = data["title"][0]
        with open("../data/" + title + ".json", 'w', encoding="utf-8") as f:
            json.dump(data, f, ensure_ascii=False, indent=4)  # JSON is superior :)
    except Exception as e:
        print("Couldn't write to file: " + str(e))


def main(url):
    """
    Main function
    """
    # Initialize the crawler
    crawler = WitcherCrawler()

    # Download the webpage
    download_webpage_as_json(crawler, url, XPATH_CONTENT)

    # Cleanup
    crawler.quit()


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <url>")
        sys.exit(1)

    main(sys.argv[1])
