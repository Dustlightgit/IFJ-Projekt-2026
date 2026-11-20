# conf.py
DOXYFILE = 'Doxyfile'

LINKS_NAVBAR1 = [
    ("Classes", 'annotated', []),
    ("Files", 'files', []),
]
LINKS_NAVBAR2 = []

SEARCH_DISABLED = False
SEARCH_DOWNLOAD_BINARY = False   # Embeds search data directly in JS so search works over file:// as well as HTTP
SEARCH_HELP = """<p>Search for files, structs, functions, or macros.</p>"""

SHOW_UNDOCUMENTED = True

EXTRA_FILES = []
PAGE_HEADER = ""
PAGE_FOOTER = ""
THEME_COLOR = "#22272e"