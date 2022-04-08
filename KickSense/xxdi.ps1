#!/usr/bin/env pwsh
# Copyright (c) 2019 Milot Midita
# 
# This program is free software: you can redistribute it and/or modify  
# it under the terms of the GNU General Public License as published by  
# the Free Software Foundation, either version 3 of the License, or 
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License 
# along with this program. If not, see http://www.gnu.org/licenses/.
#
$name = $args[0].ToLower() -replace '[^a-z0-9]','_'
$out = "unsigned char {0}[] = {{`n" -f $name
$i = 0
$bytes = Get-Content $args[0] -Encoding Byte -ReadCount 0
foreach ($b in $bytes) {
    $out += if ($i -eq 0) { "`t" } elseif ($i -lt 12) { ", " } else { $i = 0; ",`n`t" }
    $out += "0x{0:x2}" -f $b
    $i++
}
$out += "`n}};`nunsigned int {0}_len = {1};" -f $name, $bytes.Count
Write-Output $out