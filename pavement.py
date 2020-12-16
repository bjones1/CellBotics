# .. Copyright (C) 2012-2020 Bryan A. Jones.
#
#   This file is part of the CellBotics system.
#
#   The CellBotics system is free software: you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation, either version 3 of the
#   License, or (at your option) any later version.
#
#   The CellBotics system is distributed in the hope that it will be
#   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
#   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with the CellBotics system.  If not, see
#   <http://www.gnu.org/licenses/>.
#
# ***************************************
# |docname| - Configuration for Runestone
# ***************************************

import os
import sys
import pkg_resources
from socket import gethostname

from paver.easy import options, Bunch
import paver.setuputils

from runestone import get_master_url
from runestone import build  # NOQA: F401 -- build is called implicitly by the paver driver.
from runestone.server import get_dburl

paver.setuputils.install_distutils_tasks()
sys.path.append(os.getcwd())

# The project name, for use below.
project_name = os.path.basename(os.path.dirname(os.path.abspath(__file__)))

master_url = ''
if not master_url:
    master_url = get_master_url()

# The root directory for ``runestone serve``.
serving_dir = "./build/" + project_name
# The destination directory for ``runestone deploy``.
dest = "./published"

options(
    sphinx=Bunch(docroot=".",),

    build=Bunch(
        builddir=serving_dir,
        sourcedir="_sources",
        outdir=serving_dir,
        confdir=".",
        template_args={
            'login_required': 'false',
            'loglevel': 10,
            'course_title': project_name,
            'python3': 'false',
            'dburl': 'postgresql://user:password@localhost/runestone',
            'default_ac_lang': 'python',
            'jobe_server': 'http://jobe2.cosc.canterbury.ac.nz',
            'proxy_uri_runs': '/jobe/index.php/restapi/runs/',
            'proxy_uri_files': '/jobe/index.php/restapi/files/',
            'downloads_enabled': 'false',
            'enable_chatcodes': 'false',
            'allow_pairs': 'False',
            'dynamic_pages': False,
            'use_services': 'false',
            'basecourse': project_name,
            'course_id': project_name,
            # These are used for non-dynamic books.
            'appname': 'runestone',
            'course_url': master_url,
        }
    )
)

# if we are on runestone-deploy then use the proxy server not canterbury
if gethostname() == 'runestone-deploy':
    del options.build.template_args['jobe_server']
    del options.build.template_args['proxy_uri_runs']
    del options.build.template_args['proxy_uri_files']

version = pkg_resources.require("runestone")[0].version
options.build.template_args['runestone_version'] = version

# If DBURL is in the environment override dburl
options.build.template_args['dburl'] = get_dburl(outer=locals())
