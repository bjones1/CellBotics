#!/bin/sh
#
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
# **********************************************************
# publish.sh - Publish the latest github commits to the book
# **********************************************************
git pull
python3 -m runestone build --all
python3 -m runestone deploy
