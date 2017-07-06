namespace :sphinx do
  desc 'Run indexer for configured indexes'
  task :index do
    config = load_config
    if config[:indexes]
      system "#{config[:root_dir]}/indexer --config \"#{config[:config_file]}\" #{config[:indexes]}"
    else
      puts 'You should specify indexes in sphinx.yml'
    end
  end

  desc 'Run indexer for all indexes'
  task :index_all do
    config = load_config
    system "#{config[:root_dir]}/indexer --config \"#{config[:config_file]}\" --all"
  end

  desc 'Rotate configured indexes and restart searchd server'
  task :rotate do
    config = load_config
    if config[:indexes]
      system "#{config[:root_dir]}/indexer --config \"#{config[:config_file]}\" --rotate #{config[:indexes]}"
    else
      puts 'You should specify indexes in sphinx.yml'
    end
  end

  desc 'Rotate all indexes and restart searchd server'
  task :rotate_all do
    config = load_config
    system "#{config[:root_dir]}/indexer --config \"#{config[:config_file]}\" --rotate --all"
  end
  
  desc 'Start searchd server'
  task :start do
    config = load_config
    if File.exists?(config[:pid_file])
      puts 'Sphinx searchd server is already started.'
    else
      system "#{config[:root_dir]}/searchd --config \"#{config[:config_file]}\""
      puts 'Sphinx searchd server started.'
    end
  end
  
  desc 'Stop searchd server'
  task :stop do
    config = load_config
    unless File.exists?(config[:pid_file])
      puts 'Sphinx searchd server is not running.'
    else
      pid = File.read(config[:pid_file]).chomp
      kill 'SIGHUP', pid
      puts 'Sphinx searchd server stopped.'
    end
  end
  
  desc 'Restart searchd server'
  task :restart => [:stop, :start]
  
  def load_config
    return @sphinx_config if @sphinx_config

    options = YAML.load_file(File.dirname(__FILE__) + '/../../../../config/sphinx.yml') rescue {}
    @sphinx_config = {
      :config_file => options['config_file'] || '/etc/sphinx.conf',
      :root_dir => options['root_dir'] || '/usr/bin',
      :indexes => options['indexes']
    }
    sphinx_config = File.read(@sphinx_config[:config_file]) rescue ''
    
    sphinx_config =~ /searchd\s*{.*pid_file\s*=\s*(.*?)\n.*}/m
    @sphinx_config[:pid_file] = $1 || '/var/run/searchd.pid'
    return @sphinx_config
  end
end
