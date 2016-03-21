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
<div class="header">
  <h2>{{ title }}</h2>
</div>
"""


FOOTER = """
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
  background: #415192;
  border-bottom: 3px solid #aaa;
  padding: 0.8em 0.5em 0.8em;
}

.header h2 {
  color: #fefefe;
  margin: 0;
}

.container {
  max-width: 850px;
  margin: 0 auto 0;
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
<div class="container">
  <h2>References</h2>
  <p><a href="packages">Package References</a>: Documentaion for Resea packages.</p>
</div>
"""

PACKAGE_INDEX_DOC = HEADER + """
<div class="container">
  <ul>
  {% for package in packages %}
    <li><a href="{{ package['name'] }}">{{ package['name'] }} - <span class="summary">{{ package['summary'] }}</span></a></li>
  {% endfor %}
  </ul>
</div>
""" + FOOTER


PACKAGE_DOC = HEADER + """
<div class="container">
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
    {% for i in requires %}
      <li>{{ i }}</li>
    {% endfor %}
    </ul>
  </div>
</div>
""" + FOOTER

DOC = HEADER + """
{{ body }}
""" + FOOTER

