<?php
/**
 * @file cc_piwik.php
 * @brief Main CC Piwik file, to include in your code.
 * @author Romain Ducher <r.ducher@agence-codecouleurs.fr>
 * @license https://www.gnu.org/licenses/gpl-3.0.txt
 * @section LICENSE
 *
 * Copyright 2014 Code Couleurs
 *
 * This file is part of CC Piwik.
 * 
 * CC Piwik is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * CC Piwik is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * OR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * CC Piwik. If not, see <http://www.gnu.org/licenses/>.
 */

namespace cc_piwik;

/**
 * Main CC Piwik class, containing all the endpoints to call the Piwik API.
 *
 * Please refer to the Piwik Reporting API reference for further details
 * about it: http://developer.piwik.org/api-reference/reporting-api
 *
 * Note about Piwik responses with errors: they contain two fields:<ul>
 * <li>The first one is called 'result' and its value is 'error' if there are errors.</li>
 * <li>The second field is called 'message' and contains the error message.</li>
 * </ul>
 * @see http://developer.piwik.org/api-reference/reporting-api
 */
class CC_Piwik {
	####################
	# Class' internals #
	####################
	
	/** Piwik base URL */
	protected $piwik_url;
	
	/** Piwik authentication token */
	protected $token_auth;
	
	/** Format of the datas to return */
	protected $format;
	
	/** Name for the 'module' argument in URLs. */
	protected static $module = 'API';
	
	/**
	 * Constructor
	 * @param string $piwik_url Piwik base URL
	 * @param string $token_auth Piwik authentication token
	 */
    public function __construct($piwik_url, $token_auth = '', $format = 'JSON') {
		$this->set_piwik_url($piwik_url);
		$this->set_token_auth($token_auth);
		$this->set_format($format);
	}
	
	/**
	 * Executing a Piwik API request.
	 *
	 * The method :
	 * 1°) Parses the calling function name to retrieve the 'method' argument.
	 * 2°) Build the Piwik URL to ask.
	 * 3°) Returns the body of the Piwik response.
	 * @param string $function Calling CC_Piwik method's __FUNCTION__.
	 * @param array $args Calling CC_Piwik method's arguments.
	 * @return string Raw content of Piwik's response body.
	 */
	protected function ask_piwik($function, array $args) {
		return file_get_contents($this->build_endpoint_url(static::$module, $this->get_method($function), $args));
	}
	
	/**
	 * Parsing the calling function name to retrieve the 'method' argument.
	 * @param string $function  calling CC_Piwik method's __FUNCTION__.
	 * @return string "Class.Action"
	 */
	protected function get_method($function) {
		return str_replace('_', '.', $function);
	}
	
	/**
	 * Building the Piwik URL to ask with its GET arguments.
	 * @param array $get_args GET arguments
	 * @return string The complete URL (to use for asking Piwik some datas).
	 */
	protected function build_piwik_url(array $get_args) {
		$query_args = array();
		foreach ($get_args as $name => $value) {
			$query_args[] = $name.'='.urlencode($value);
		}
		
		return $this->piwik_url.'?'.implode('&', $query_args);
	}
	
	/**
	 * Building the Piwik URL to ask.
	 * @param string $module 'module' Piwik API's argument
	 * @param string $method 'method' Piwik API's argument
	 * @param array $other_args Other endpoint arguments
	 * @return string The URL to use for asking Piwik some datas.
	 */
	protected function build_endpoint_url($module, $method, array $other_args) {
		$get_args = array_merge(
			$other_args,
			array(
				'module'     => $module,
				'method'     => $method,
				'format'     => $this->format,
				'token_auth' => $this->token_auth,
			)
		);
		
		return $this->build_piwik_url($get_args);
	}
	
	
	#######################
	# Getters and setters #
	#######################
	
	// piwik_url
	
	/**
	 * Getter for $this->piwik_url.
	 * @return $this->piwik_url
	 */
	public function get_piwik_url() {
		return $this->piwik_url;
	}
	
	/*
	 * Setter for $this->piwik_url.
	 * @param string $new_value New value for $this->piwik_url.
	 */
	public function set_piwik_url($new_value) {
		$this->piwik_url = $new_value;
	}
	
	// token_auth
	
	/**
	 * Getter for $this->token_auth.
	 * @return $this->token_auth
	 */
	public function get_token_auth() {
		return $this->token_auth;
	}
	
	/*
	 * Setter for $this->token_auth.
	 * @param string $new_value New value for $this->.token_auth
	 */
	public function set_token_auth($new_value) {
		$this->token_auth = $new_value;
	}
	
	/**
	 * Setting the token_auth by asking it to the Piwik API.
	 * @param string $userLogin User login
	 * @param string $password User's password clear or encrypted with md5.
	 * @param bool $password_is_clear true if $password is clear,
	 * false if it is encrypted with md5.
	 */
	public function set_token_auth_from_credentials($userLogin, $password, $password_is_clear = true) {
		$cc_piwik = clone $this;
		$cc_piwik->set_format('JSON');
		$piwik_res = json_decode($cc_piwik->usersManager_getTokenAuth($userLogin, $password_is_clear ? md5($password) : $password));
		if (isset($piwik_res->value)) {
			$this->set_token_auth($piwik_res->value);
		}
	}
	
	// format
	
	/**
	 * Getter for $this->format.
	 * @return $this->format
	 */
	public function get_format() {
		return $this->format;
	}
	
	/*
	 * Setter for $this->format.
	 * @param string $new_value New value for $this->format.
	 */
	public function set_format($new_value) {
		$this->format = $new_value;
	}
	
	
	#########
	# Logme #
	#########
	
	/**
	 * Building an URL in order to log into Piwik using the 'logme' feature
	 * ( http://piwik.org/faq/how-to/faq_30/ ).
	 * @param string $login Login
	 * @param string $password Password (clear or MD5)
	 * @param string $redirect_url (Optional) URL to redirect after logging into Piwik.
	 * @param int $idSite (Optional) Website ID on Piwik.
	 * @param bool $password_is_clear (Optional) true if $password is the real
	 * password value, false if $password is the password's MD5.
	 * @return string The URL to log into Piwik
	 */
	public function get_logme_url($login, $password, $redirect_url = '', $idSite = 0, $password_is_clear = true) {
		$get_args = array(
			'module'   => 'Login',
			'action'   => 'logme',
			'login'    => $login,
			'password' => $password_is_clear ? md5($password) : $password
		);
		
		if (!empty($redirect_url)) {
			$get_args['url'] = $redirect_url;
		}
		
		if (!empty($idSite)) {
			$get_args['idSite'] = intval($idSite);
		}
		
		return $this->build_piwik_url($get_args);
	}
	
	
	#######################
	# Module SitesManager #
	#######################
	
	public function SitesManager_getAllSites() {
		return $this->ask_piwik(__FUNCTION__, array());
	}
	
	public function SitesManager_getAllSitesId() {
		return $this->ask_piwik(__FUNCTION__, array());
	}
	
	public function SitesManager_getSitesIdFromSiteUrl($url) {
		return $this->ask_piwik(__FUNCTION__, array('url' => $url));
	}
	
	
	#######################
	# Module UsersManager #
	#######################
	
	/**
	 * Creating a Piwik user.
	 * @param string $userLogin Login of the brand new.
	 * @param string $password Its password (clear)
	 * @param string $email Its email
	 * @param string $alias Its (optional) alias
	 * @return string A reply looking like this (in JSON) :
	 * {'result': 'success|error', 'message': <message for the result>}
	 */
	public function UsersManager_addUser($userLogin, $password, $email, $alias = '') {
		$args = array(
			'userLogin' => $userLogin,
			'password'  => $password,
			'email'     => $email
		);
		
		if (!empty($alias)) {
			$args['alias'] = $alias;
		}
		
		return $this->ask_piwik(__FUNCTION__, $args);
	}
	
	public function UsersManager_getUser($userLogin) {
		return $this->ask_piwik(__FUNCTION__, array('userLogin' => $userLogin));
	}
	
	/**
	 * Updating a Piwik user.
	 *
	 * All arguments are optional except $userLogin.
	 * @param string $userLogin Login of the user to update
	 * @param string $password New password (clear)
	 * @param string $email New email
	 * @param string $alias New alias
	 * @return string A reply looking like this (in JSON) :
	 * {'result': 'success|error', 'message': <message for the result>}
	 */
	public function UsersManager_updateUser($userLogin, $password = '', $email = '', $alias = '') {
		$args = array('userLogin' => $userLogin);
		
		if (!empty($password)) {
			$args['password'] = $password;
		}
		
		if (!empty($email)) {
			$args['email'] = $email;
		}
		
		if (!empty($alias)) {
			$args['alias'] = $alias;
		}
		
		return $this->ask_piwik(__FUNCTION__, $args);
	}
	
	/**
	 * Deleting a Piwik user
	 * @param string $userLogin Login of the user to delete
	 * @return string A reply looking like this (in JSON) :
	 * {'result': 'success|error', 'message': <message for the result>}
	 */
	public function UsersManager_deleteUser($userLogin) {
		return $this->ask_piwik(__FUNCTION__, array('userLogin' => $userLogin));
	}
	
	public function UsersManager_getTokenAuth($userLogin, $md5Password) {
		return $this->ask_piwik(__FUNCTION__, array('userLogin' => $userLogin, 'md5Password' => $md5Password));
	}
	
	/**
	 * Managing user accesses to websites
	 * @param string $userLogin Login of the user to set rights
	 * @param string $access Kind of access : 'view' for consulting datas,
	 * 'admin' to administrate the website on Piwik or 'noaccess' for no accesses
	 * to the website.
	 * @param array $idSites Sites for setting the user accesses
	 * @return string The Reporting Piwik API's JSON reply.
	 */
	public function UsersManager_setUserAccess($userLogin, $access, array $idSites) {
		return $this->ask_piwik(__FUNCTION__, array(
			'userLogin' => $userLogin,
			'access'    => $access,
			'idSites'   => implode(',', $idSites)
		));
	}
	
	/**
	 * 
	 * @param string $access Kind of access : 'view' for consulting datas,
	 * 'admin' to administrate the website on Piwik or 'noaccess' for no accesses
	 * to the website.
	 * @return string The Reporting Piwik API's reply. It is a list containing
	 * objects which looks like {userLogin: [list of idSites]} (in JSON).
	 */
	public function UsersManager_getUsersSitesFromAccess($access) {
		return $this->ask_piwik(__FUNCTION__, array('access' => $access));
	}
	
	/**
	 * 
	 * @param int $idSite Website ID
	 * @return string The Reporting Piwik API's reply. It is a list
	 * containing objects which looks like {userLogin: access} (in JSON).
	 */
	public function UsersManager_getUsersAccessFromSite($idSite) {
		return $this->ask_piwik(__FUNCTION__, array('idSite' => intval($idSite)));
	}
	
	/**
	 * 
	 * @param int $idSite Website ID
	 * @param string $access Kind of access : 'view' for consulting datas,
	 * 'admin' to administrate the website on Piwik or 'noaccess' for no accesses
	 * to the website.
	 * @return string The Reporting Piwik API's reply. It is a list of
	 * Piwik users with all their Piwik datas in objects.
	 */
	public function UsersManager_getUsersWithSiteAccess($idSite, $access) {
		return $this->ask_piwik(__FUNCTION__, array(
			'idSite' => intval($idSite),
			'access' => $access
		));
	}
	
	/**
	 * 
	 * @param string $userLogin Login of the user
	 * @return string The Reporting Piwik API's reply. It is a list of objects
	 * whick looks like : {'site': idSite , 'access': theAccess} (in JSON).
	 */
	public function UsersManager_getSitesAccessFromUser($userLogin) {
		return $this->ask_piwik(__FUNCTION__, array('userLogin' => $userLogin));
	}
	
	/**
	 * Checks if an user identified by the login $userLogin exists on Piwik.
	 * @param string $userLogin Login of the user
	 * @return string The Reporting Piwik API's reply. It is an object with a
	 * unique field called "value" which is equal to true or false, depending on
	 * if the user exists or not.
	 */
	public function UsersManager_userExists($userLogin) {
		return $this->ask_piwik(__FUNCTION__, array('userLogin' => $userLogin));
	}
}
