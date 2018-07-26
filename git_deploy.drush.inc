<?php

/**
 * Implements hook_drush_command().
 */
function git_deploy_drush_command() {
  $items = array();
  $items['git-deploy-fetch'] = array(
    'description' => 'Forces a fresh fetch of all the module / theme informations.',
    'callback' => 'drush_git_deploy_fetch',
    'examples' => array(
      'drush git-deploy-fetch' => 'Fetch all info of all modules / themes.',
      'drush git-deploy-fetch views' => 'Fetch all info of the views module.',
    ),
    'arguments' => array(
      'module' => 'The name of a module to process',
    ),
  );

  return $items;
}

function drush_git_deploy_fetch($module = NULL) {
  drush_log(dt('Recreating queue.'), 'success');
  $queue = DrupalQueue::get('git_deploy_update_version_info');
  $queue->deleteQueue();
  // Get all the modules.
  $modules = system_list('module_enabled');
  $themes = system_list('theme');
  if (empty($module)) {
    foreach ($modules as $module) {
      $queue->createItem($module);
    }
    foreach ($themes as $theme) {
      $queue->createItem($theme);
    }
  }
  else {
    if (isset($modules[$module])) {
      drush_log(dt('Found module @module - added to queue.', array('@module' => $module)), 'success');
      $queue->createItem($modules[$module]);
    }
    if (isset($themes[$module])) {
      drush_log(dt('Found theme @module - added to queue.', array('@module' => $module)), 'success');
      $queue->createItem($themes[$module]);
    }
  }

  /** @var DrupalQueueInterface $queue */
  $queue = DrupalQueue::get('git_deploy_update_version_info');
  if (($queue_items = $queue->numberOfItems())) {
    drush_log(dt('Processing batch with !items items.', array('!items' => $queue_items)), 'success');
    // Set the batch operations.
    $batch = array(
      'title' => dt('Processing module / theme'),
      'operations' => array(),
    );
    for ($i = 1; $i <= $queue_items; $i++) {
      $batch['operations'][] = array('drush_git_deploy_fetch_process_item', array(1));
    }
    batch_set($batch);
    drush_backend_batch_process();
  }
}

function drush_git_deploy_fetch_process_item() {
  /** @var DrupalQueueInterface $queue */
  $queue = DrupalQueue::get('git_deploy_update_version_info');
  if ($item = $queue->claimItem()) {
    drush_log(dt('Processing: @item', array('@item' => $item->data->name)), 'success');
    $module = git_deploy_update_version_info_worker($item->data);
    drush_log(dt('        detected version: @version', array('@version' => $module->info['version'])), 'success');
    $queue->deleteItem($item);
  }
}