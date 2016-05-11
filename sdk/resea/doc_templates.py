HEADER = """
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>{{ title }}</title>
{% if description is defined %}
  <meta name="description" content="{{ summary }}">
{% endif %}
{% if css %}
  <style>{{ css }}</style>
{% endif %}
</head>
<body>
<div class="container">
<div class="header">
  <h1><img src="http://resea.net/resea.svg" alt="logo"><span>{{ title }}</span></h1>
  <p class="revision">{{ revision }}</p>
</div>
<div class="body">
"""


FOOTER = """
</div>
</div>
</body>
</html>
"""


COMMON_CSS = """
body {
  font-family: 'Lucida Grande', sans-serif;
  color: #555;
  margin: 0;
}

.header {
  border-bottom: 1px solid #555;
  padding-bottom: 10px;
}

.header h1 {
  display: inline;
  margin: 0;
}

.header h1 img {
  position: relative;
  top: 5px;
  height: 40px;
}

.header h1 span {
  margin-left: 20px;
  font-size: 25px;
  font-family: "Source Code Pro", sans-serif;
  font-weight: 400;
}

.revision {
  display: inline;
  float: right;
  margin-top: 20px;
  margin-bottom: 10px;
}

.header:after {
  clear: both;
}

.container {
  max-width: 850px;
  margin: 0 auto 0;
  padding: 0 50px 0;
}

.body {
  padding: 0 30px 0;
  color: #333;
  line-height: 28px;
  font-size: 16px;
}

h2 {
  margin-top: 70px;
  margin-bottom: 40px;
  font-weight: normal;
}

pre code:only-child {
  padding: 20px;
  display: block;
  border-left: #e5e5e5 5px solid;
  white-space: pre;
}

code {
  background: #f8f8f8;
  line-height: 19px;
  font-size: 13px;
}

"""

PACKAGE_DOC_CSS = COMMON_CSS + """
.markdown h1 {
  display: none;
}

.description {
  font-size: 120%;
  font-weight: 400;
  color: #666;
}

.main {
  float: left;
  width: 65%:
}

.readme {
  margin-top: 3em;
}

.sidebar {
  float: right;
  width: 30%;
  margin-left: 30px;
  font-size: 95%;
}

h3 {
  margin-top: 3em;
  font-size: 115%;
}

.sidebar ul {
  margin: 0;
  padding: 0;
  list-style-type: none;
}

.sidebar li {
  padding: 0.7em 0 0.7em;
}

.sidebar li:not(:last-child) {
  border-bottom: 1px solid #aaa;
}

.category {
  text-transform: capitalize;
}
"""


PACKAGE_INDEX_DOC_CSS = COMMON_CSS + """
.summary {
  color: #aaa;
}

li {
  margin-bottom: 1em;
}

a {
  text-decoration: none;
}
"""

INDEX_DOC_CSS = COMMON_CSS + """
"""

INDEX_DOC = HEADER + """
<h2>Tutorial</h2>
<p><a href="http://resea.net/resea-by-example">Resea by Example</a>: Learn Resea by Examples</p>

<h2>References</h2>
<p><a href="packages">Package References</a>: Documentaion for Resea packages.</p>

<h2>Specifications</h2>
<p><a href="thread.html">Thread and Thread Group</a>: A execution unit and set of threads.</p>
<p><a href="messaging.html">Messaging</a>: A inter-thread communication.</p>
<p><a href="package.html">Package and Executable</a>: A isolated component.</p>

<h2>SDK</h2>
<p><a href="commands.html">Commands</a>: How to use SDK.</p>
<p><a href="package.yml.html">package.yml</a>: The essential file for a package.</p>
"""

PACKAGE_INDEX_DOC = HEADER + """
<ul>
{% for package in packages %}
  <li><a href="{{ package['name'] }}">{{ package['name'] }} - <span class="summary">{{ package['summary'] }}</span></a></li>
{% endfor %}
</ul>
""" + FOOTER


PACKAGE_DOC = HEADER + """
<div class="main">
  <h1 class="name">{{ name }}</h1>
  <p class="description">{{ summary }}</p>
  <div class="readme markdown">
    {{ readme }}
  </div>
</div>

<div class="sidebar">
  <h3>Profile</h3>
  <ul>
    <li class="category">{{ category }}</li>
    <li>created by {{ author }}</li>
    <li>{{ license }}</li>
    <li><a href="{{ homepage }}">{{ homepage or '' }}</a></li>
  </ul>
  
  <h3>Implements</h3>
  <ul>
  {% for i in implements %}
    <li>{{ i }}</li>
  {% endfor %}
  </ul>

  <h3>Uses</h3>
  <ul>
  {% for i in uses %}
    <li>{{ i }}</li>
  {% endfor %}
  </ul>

  <h3>Requires</h3>
  <ul>
  {% for i in depends %}
    <li>{{ i }}</li>
  {% endfor %}
  </ul>
</div>
""" + FOOTER

DOC = HEADER + """
{{ body }}
""" + FOOTER

