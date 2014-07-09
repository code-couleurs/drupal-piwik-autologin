<?php
/**
 * @file
 * piwik_autologin.utils.inc
 * @brief Some utilities for the module.
 * @author Romain Ducher <r.ducher@agence-codecouleurs.fr>
 * @license https://www.gnu.org/licenses/gpl-3.0.txt
 * @section LICENSE
 *
 * Copyright 2014 Code Couleurs
 *
 * This file is part of Piwik Autologin.
 *
 * Piwik Autologin is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Piwik Autologin is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS OR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Piwik Autologin. If not, see <http://www.gnu.org/licenses/>.
 */

namespace piwik_autologin;

if (!class_exists('\piwik_autologin\PiwikUser')) {
  require_once 'piwik_autologin.utils.inc';
}

/**
 * Retrieving the URL of the Piwik which tracks the website.
 *
 * @return string
 *   The corresponding Piwik URL.
 */
function get_piwik_url() {
  return variable_get('piwik_autologin_piwik_url', '');
}

/**
 * Setting a new value for the URL of the Piwik which tracks the website.
 *
 * @param string $new_url
 *   The new URL.
 */
function set_piwik_url($new_url) {
  return variable_set('piwik_autologin_piwik_url', $new_url);
}

/**
 * Retrieving the Website ID of the site on the Piwik install which tracks the site.
 *
 * @return int
 *   The corrstonding website ID.
 */
function get_piwik_idsite() {
  return variable_get('piwik_autologin_idsite', 0);
}

/**
 * Setting a new value for the Website ID of the site on the Piwik install which tracks the site.
 *
 * @return int
 *   The new website ID.
 */
function set_piwik_idsite($new_idsite) {
  return variable_set('piwik_autologin_idsite', intval($new_idsite));
}

/**
 * Redirect to Piwik.
 */
function go_to_piwik() {
  header('Location: ' . get_piwik_url());
  exit;
}

/**
 * Getting stuff to communicate with the Piwik API.
 *
 * This function is based on CC Piwik.
 *
 * @see http://github.com/code-couleurs/cc_piwik
 * @return \cc_piwik\CC_Piwik
 *   A CC Piwik instance.
 */
function get_piwik_instance() {
  if (!class_exists('\cc_piwik\CC_Piwik')) {
    require_once 'libs/cc_piwik/cc_piwik.php';
  }

  return new \cc_piwik\CC_Piwik(get_piwik_url());
}

/**
 * Knowing if the link to Piwik has to be shown in the navigation menu.
 *
 * @return bool
 *   TRUE if it has to be shown, FALSE otherwise.
 */
function get_show_piwik_navigation_link() {
  return (isset($_SESSION['show_piwik_navi_link']) AND $_SESSION['show_piwik_navi_link']);
}

/**
 * Setting a new value for determining if the link to Piwik has to be shown in the navigation menu.
 *
 * @param bool $newval
 *   New value for showing the link to Piwik in the navigation menu. TRUE if the
 *   link has to be shown, FALSE otherwise.
 */
function set_show_piwik_navigation_link($newval) {
  $_SESSION['show_piwik_navi_link'] = $newval;
}
