# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
sys.path.insert(0, os.path.abspath(''))
sys.path.append(os.path.abspath('./_ext'))


# -- Project information -----------------------------------------------------

project = 'Element Documentation'
copyright = '2020, Ultraleap'
author = 'Ultraleap'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ["breathe",
              "examplecode",
              "sphinx_csharp",
              "sphinx_rtd_theme"]

# Breathe Configuration
breathe_default_project = "ElementDotNET"

sphinx_csharp_multi_language = True

# Setup the exhale extension
#exhale_args = {
#    # Required arguments
#    "containmentFolder": "./api",
#    "rootFileName": "library_root.rst",
#    "rootFileTitle": "Library API",
#    "doxygenStripFromPath":  "..",
#    # Suggested optional arguments
#    "createTreeView":        True,
#    # TIP: if using the sphinx-bootstrap-theme, you need
#    "treeViewIsBootstrap": True
#}

primary_domain = 'cpp'

highlight_language = 'cpp'

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store', 'cmake', 'build']

# The master toctree document.
master_doc = 'index'

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.

import sphinx_rtd_theme
html_theme = 'sphinx_rtd_theme'

html_sidebars = {'**': ['localtoc.html', 'searchbox.html']}

html_logo = "img/element.png"

mater_doc = 'index'

html_theme_options = {
    'logo_only': True,
    'style_nav_header_background': "#EA7601"
}

html_favicon = 'img/favicon.ico'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['static']

html_css_files = [
    'theme.css'
]


def setup(app):
    app.add_css_file('theme.css')

