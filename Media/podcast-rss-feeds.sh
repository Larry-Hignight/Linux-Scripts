CMD="podfox import"

echo "Importing BBC and NPR Radio Shows"
$CMD http://podcasts.files.bbci.co.uk/b00srz5b.rss               # Mathematics
$CMD http://podcasts.files.bbci.co.uk/p002vsxs.rss               # Business Daily
$CMD http://feeds.wnyc.org/radiolab                              # Radiolab
$CMD https://www.sciencefriday.com/feed/podcast/podcast-episode  # Science Friday
$CMD https://www.npr.org/templates/rss/podcast.php?id=510298     # TED Radio Hour
$CMD http://podcasts.files.bbci.co.uk/p05h5sw6.rss               # Tomorrow's World

echo "Importing Data Science, Machine Learning and AI Podcasts"
$CMD https://dataskeptic.com/api/blog/rss                        # Data Skeptic
$CMD https://r-podcast.org/episode/index.xml                     # R Podcast
$CMD http://feeds.podtrac.com/IOJSwQcdEBcg                       # O'Reilly Data Show
$CMD http://feeds.feedburner.com/PartiallyDerivative             # Partially Derivative
$CMD https://concerning.ai/feed/                                 # Concerning AI
# Linear Digressions   ???

echo "Importing UAS Related Podcasts"
$CMD http://theuavdigest.com/feed/podcast/                       # UAV Digest
$CMD http://www.blogtalkradio.com/suasnews/podcast               # sUAS News
